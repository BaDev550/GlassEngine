#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Core/Window.h"
#include "GlassEngine/Memory/Memory.h"
#include "GlassEngine/Renderer/Buffer.h"
#include "GlassEngine/Renderer/Swapchain.h"
#include "GlassEngine/Renderer/Types.h"
#include "Platform/Vulkan/Vulkan_RenderContext.h"
#include "Platform/Vulkan/Vulkan_Types.h"
#include "gepch.h"
#include "Vulkan_RenderAPI.h"
#include "Vulkan_Swapchain.h"
#include "Vulkan_Image.h"
#include "Vulkan_Buffer.h"
#include "Vulkan_DescriptorManager.h"
#include "Vulkan_Sampler.h"
#include "Vulkan_Pipeline.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace ge::renderer {
	namespace utility {
		constexpr VkPipelineStageFlags2 Vulkan_GetPipelineStageFlagsFromLayout(VkImageLayout layout) noexcept {
			switch (layout) {
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				return VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				return VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				return VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			case VK_IMAGE_LAYOUT_GENERAL:
				return VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				return VK_PIPELINE_STAGE_2_TRANSFER_BIT;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				return VK_PIPELINE_STAGE_2_TRANSFER_BIT;
			default:
				return {};
			}
		}
	
		constexpr VkAccessFlags2 Vulkan_GetAccessFlagsFromLayout(VkImageLayout layout, bool loadOpIsLoad = false) noexcept {
			switch (layout) {
				case VK_IMAGE_LAYOUT_GENERAL: 
					return VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
					return !loadOpIsLoad ? VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT : VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
					return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
					return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
					return VK_ACCESS_2_SHADER_READ_BIT;
				case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
					return VK_ACCESS_2_TRANSFER_READ_BIT;
				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
					return VK_ACCESS_2_TRANSFER_WRITE_BIT;
				default: return {};
			}
		}
	}

	VkImageMemoryBarrier2 Vulkan_RenderAPI::GetMemoryBarrier(Vulkan_Image& image, const VkImageLayout newImageLayout, bool loadOpIsLoad) {
		const auto oldImageLayout = image.GetImageLayout();
		image._imageLayout = newImageLayout;
		return {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			nullptr,
			utility::Vulkan_GetPipelineStageFlagsFromLayout(oldImageLayout),
			utility::Vulkan_GetAccessFlagsFromLayout(oldImageLayout),
			utility::Vulkan_GetPipelineStageFlagsFromLayout(newImageLayout),
			utility::Vulkan_GetAccessFlagsFromLayout(newImageLayout),
			oldImageLayout,
			newImageLayout,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			image.GetImage(),
			VkImageSubresourceRange{
				.aspectMask = image.GetAspectFlags(),
				.baseMipLevel = 0,
				.levelCount = VK_REMAINING_MIP_LEVELS,
				.baseArrayLayer = 0,
				.layerCount =  VK_REMAINING_ARRAY_LAYERS
			}
		};
	}

	Vulkan_RenderAPI::Vulkan_RenderAPI()
	{
		for (uint32_t i = 0; i < Renderer3D::MaxFramesInFlight; i++) { // TEMP
			auto& frame = _frames[i];

			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = VK_RENDER_CONTEXT->GetGraphicsQueueFamilyIndex();
			vkCreateCommandPool(VK_RENDER_CONTEXT->GetDevice(), &poolInfo, VK_ALLOCATOR_CALLBACKS, &frame.commandPool);

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandBufferCount = 1;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = frame.commandPool;
			vkAllocateCommandBuffers(VK_RENDER_CONTEXT->GetDevice(), &allocInfo, &frame.commandBuffer);
		}
	}

	Vulkan_RenderAPI::~Vulkan_RenderAPI()
	{
		for (uint32_t i = 0; i < Renderer3D::MaxFramesInFlight; i++) {
			vkFreeCommandBuffers(VK_RENDER_CONTEXT->GetDevice(), _frames[i].commandPool, 1, &_frames[i].commandBuffer);
			vkDestroyCommandPool(VK_RENDER_CONTEXT->GetDevice(), _frames[i].commandPool, VK_ALLOCATOR_CALLBACKS);
		}
	}

	void Vulkan_RenderAPI::PushConstant(const void *ptr, uint16_t size, uint16_t offset) {
		VK_RENDER_CONTEXT->GetDescriptorManager().PushConstant(GetCurrentCommandBuffer(), ptr, size, offset);
	}

	void Vulkan_RenderAPI::BeginFrame()
	{
		GE_ASSERT(!_frameStarted, "Cannot call beginFrame while processing a frame");
		_frameStarted = true;
		uint32_t frameIndex = Renderer3D::GetFrameIndex();
		FrameContext& frame = _frames[frameIndex];

		Application::Get()->GetWindow().Swapbuffers();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkResetCommandPool(VK_RENDER_CONTEXT->GetDevice(), frame.commandPool, 0);
		vkBeginCommandBuffer(frame.commandBuffer, &beginInfo);

		VK_RENDER_CONTEXT->GetDescriptorManager().BindDescriptors(frame.commandBuffer);

		auto& window = Application::Get()->GetWindow();
		const auto &swapchain = static_cast<Vulkan_Swapchain &>(window.GetSwapchain());
		const auto image = swapchain.GetImages()[window.GetImageIndex()];

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		barrier.oldLayout = frameIndex <= swapchain.GetImages().size() ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(
			frame.commandBuffer,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	void Vulkan_RenderAPI::EndFrame()
	{
		GE_ASSERT(_frameStarted, "Cannot call endFrame while not processing a frame");
		VkCommandBuffer cmd = GetCurrentCommandBuffer();
		auto& window = Application::Get()->GetWindow();
		auto &swapchain = static_cast<Vulkan_Swapchain &>(window.GetSwapchain());
		uint32_t frameIndex = Renderer3D::GetFrameIndex();
		uint32_t imageIndex = window.GetImageIndex();

		const auto image = swapchain.GetImages()[window.GetImageIndex()];

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = {};
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(
			cmd,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		vkEndCommandBuffer(cmd);
		VkResult result = swapchain.Submit(&cmd, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.HasResized()) {
			window.ResetResizeFlag();
			SwapchainSpec newSpec = swapchain.GetSpecs();
			newSpec.extent.x = window.GetWidth();
			newSpec.extent.y = window.GetHeight();
			swapchain.ReCreateSwapchain(newSpec);
		}
		_renderStats.Reset();
		_frameStarted = false;
	}

	void Vulkan_RenderAPI::Draw(ge::mem::Ref<Pipeline>& pipeline, uint32_t vertexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, uint32_t firstVertex, uint32_t firstInstance)
	{
		auto vulkanVertexBuffer = vertexBuffer.Cast<Vulkan_Buffer>()->GetVkBuffer();
		auto vulkanPipeline = pipeline.Cast<Vulkan_Pipeline>()->GetPipeline();

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindPipeline(GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline);
		vkCmdBindVertexBuffers(GetCurrentCommandBuffer(), 0, 1, &vulkanVertexBuffer, offsets);
		vkCmdDraw(GetCurrentCommandBuffer(), vertexCount, instanceCount, firstVertex, firstInstance);
		_renderStats.drawCalls++;
	}

	void Vulkan_RenderAPI::DrawIndexed(ge::mem::Ref<Pipeline>& pipeline, uint32_t indexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, ge::mem::Ref<Buffer> indexBuffer, uint32_t firstIndex, uint32_t firstInstance, int32_t vertexOffset)
	{
		auto vulkanVertexBuffer = vertexBuffer.Cast<Vulkan_Buffer>()->GetVkBuffer();
		auto vulkanIndexBuffer = indexBuffer.Cast<Vulkan_Buffer>()->GetVkBuffer();
		auto vulkanPipeline = pipeline.Cast<Vulkan_Pipeline>()->GetPipeline();

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindPipeline(GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline);
		vkCmdBindVertexBuffers(GetCurrentCommandBuffer(), 0, 1, &vulkanVertexBuffer, offsets);
		vkCmdBindIndexBuffer(GetCurrentCommandBuffer(), vulkanIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(GetCurrentCommandBuffer(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
		_renderStats.drawCalls++;
	}

	void Vulkan_RenderAPI::DrawStaticMesh(ge::mem::Ref<Pipeline>& pipeline, ge::mem::Ref<StaticMesh>& mesh, uint32_t lodIndex, ge::mem::Ref<MaterialTable> materialTable, const glm::mat4& transform)
	{
		const auto& currentLOD = mesh->GetLODs()[lodIndex];
		auto& materialAssets = materialTable ? materialTable->GetMaterials() : mesh->GetMaterialTable()->GetMaterials();
		auto vulkanPipeline = pipeline.Cast<Vulkan_Pipeline>()->GetPipeline();
		auto vulkanVertexBuffer = mesh->GetVertexBuffer().Cast<Vulkan_Buffer>()->GetVkBuffer();
		auto vulkanIndexBuffer = mesh->GetIndexBuffer().Cast<Vulkan_Buffer>()->GetVkBuffer();

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindPipeline(GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline);
		vkCmdBindVertexBuffers(GetCurrentCommandBuffer(), 0, 1, &vulkanVertexBuffer, offsets);
		vkCmdBindIndexBuffer(GetCurrentCommandBuffer(), vulkanIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
		PushConstant(&transform, sizeof(glm::mat4), 0);
		for (const Submesh& submesh : currentLOD.submesh) {
			auto materialAsset = materialAssets[submesh.materialIndex];
			if (materialAsset) {
				auto material = materialAsset->GetMaterial();
				PushConstant(&material->GetBindlessData(), sizeof(MaterialBindlessData), sizeof(glm::mat4));
			}

			vkCmdDrawIndexed(GetCurrentCommandBuffer(), submesh.indexCount, 1, submesh.indexOffset, submesh.vertexOffset, 0);
			_renderStats.drawCalls++;
		}
	}

	void Vulkan_RenderAPI::BeginCopyPass() {

	}

	void Vulkan_RenderAPI::EndCopyPass() {
		Barrier();
	}

	VkCommandBuffer Vulkan_RenderAPI::GetCurrentCommandBuffer() {
		GE_ASSERT(_frameStarted, "Cannot get active command buffer while frame is not started");
		return _frames[Renderer3D::GetFrameIndex()].commandBuffer;
	}

	void Vulkan_RenderAPI::BeginRenderPass(const BeginRenderPassSpec& spec) {
		// Barrier();

		VkCommandBuffer cmd = GetCurrentCommandBuffer();
		
		const auto& window = Application::Get()->GetWindow();
		const uint32_t imageIndex = window.GetImageIndex();

		GEVector<VkRenderingAttachmentInfo> colorAttachments;
		colorAttachments.reserve(spec.colorAttachments.size());
		colorAttachments.reserve(spec.colorAttachments.size());

		GEVector<VkImageMemoryBarrier2> memBarriers;
		memBarriers.reserve(spec.colorAttachments.size() + 1);
		memBarriers.reserve(spec.colorAttachments.size() + 1);

		for (const auto& attachment : spec.colorAttachments) {
			auto vk_image = attachment.image.Cast<Vulkan_Image>();

			if (!attachment.isSwapchainImage && vk_image->GetImageLayout() != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
				memBarriers.emplace_back(
					GetMemoryBarrier(
						*vk_image, 
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						attachment.loadOp == AttachmentLoadOp::Load
					)
				);
				_barriersForImages.emplace(vk_image.Get());
			}

			colorAttachments.emplace_back(
				VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				nullptr,
				attachment.isSwapchainImage ? 
					static_cast<const Vulkan_Swapchain&>(window.GetSwapchain()).GetImageViews()[imageIndex]
					: vk_image->CreateGetImageView(attachment.subresource),
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VkResolveModeFlagBits{},
				VkImageView{},
				VkImageLayout{},
				utility::Vulkan_GetLoadOp(attachment.loadOp),
				utility::Vulkan_GetStoreOp(attachment.storeOp),
				VkClearValue{.color{
					attachment.clearValue.colorClear.r, 
					attachment.clearValue.colorClear.g, 
					attachment.clearValue.colorClear.b, 
					attachment.clearValue.colorClear.a
				}}
			);
		}

		auto vk_depthStencilImage = spec.depthStencilAttachment.image.Cast<Vulkan_Image>();
		const auto has_depth = vk_depthStencilImage != nullptr;
		const auto has_stencil = has_depth ? 
			utility::IsDepthStencilFormat(vk_depthStencilImage->GetSpecRef().imageFormat) : false;

		if (has_depth && vk_depthStencilImage->GetImageLayout() != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			memBarriers.emplace_back(
				GetMemoryBarrier(
					*vk_depthStencilImage, 
					VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				)
			);
		}
		if (has_depth) _barriersForImages.emplace(vk_depthStencilImage.Get());

		const VkRenderingAttachmentInfo depthStencilAttachment
		 	= !has_depth ? VkRenderingAttachmentInfo{} : VkRenderingAttachmentInfo{
			VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			nullptr,
			vk_depthStencilImage->CreateGetImageView(spec.depthStencilAttachment.subresource),
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VkResolveModeFlagBits{},
			VkImageView{},
			VkImageLayout{},
			utility::Vulkan_GetLoadOp(spec.depthStencilAttachment.loadOp),
			utility::Vulkan_GetStoreOp(spec.depthStencilAttachment.storeOp),
			VkClearValue{.depthStencil{
				spec.depthStencilAttachment.clearValue.depthClear,
				spec.depthStencilAttachment.clearValue.stencilClear
			}}
		};

		VkDependencyInfo dependencyInfo{};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pImageMemoryBarriers = memBarriers.data();
		dependencyInfo.imageMemoryBarrierCount = memBarriers.size();

		vkCmdPipelineBarrier2(GetCurrentCommandBuffer(), &dependencyInfo);

		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = VkRect2D({ 0, 0 }, {spec.extent.x, spec.extent.y});
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
		renderingInfo.pColorAttachments = colorAttachments.data();
		if (has_depth) renderingInfo.pDepthAttachment = &depthStencilAttachment;
		if (has_stencil) renderingInfo.pStencilAttachment = &depthStencilAttachment;
		
		VkViewport viewport{ 0, 0, (float)spec.extent.x, (float)spec.extent.y };
		viewport.minDepth = 0;
		viewport.maxDepth = 1;
		const VkRect2D scissor{ {0,0}, {spec.extent.x, spec.extent.y} };

		vkCmdBeginRendering(cmd, &renderingInfo);
		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdSetScissor(cmd, 0, 1, &scissor);
	}

	void Vulkan_RenderAPI::EndRenderPass() {
		VkCommandBuffer cmd = Renderer3D::GetRenderAPI().Cast<Vulkan_RenderAPI>()->GetCurrentCommandBuffer();
		vkCmdEndRendering(cmd);
		Barrier();
	}

	void Vulkan_RenderAPI::ILoadDataToBuffer(const ge::mem::Ref<Buffer>& buffer, const void* data, uint64_t dataSize) {
		const auto cmd = GetCurrentCommandBuffer();
		if (buffer->GetSpecRef().cpuAccess != BufferCpuAccess::None) {
			std::memcpy(buffer->GetMappedPtr(), data, dataSize);
			return;
		}
		else if (dataSize < (1u<<20) * 64) {
			vkCmdUpdateBuffer(cmd, 
				buffer.Cast<Vulkan_Buffer>()->GetVkBuffer(), 0, dataSize, data);
			return;
		}
		
		const auto &staging_buffer = staging_buffers.emplace_back(ge::mem::CreateScope<Vulkan_Buffer>(BufferSpec{
			.elementSize = static_cast<uint32_t>(dataSize),
			.elementCount = 1,
			.usageFlags = BufferUsageFlagsBits::TransferSrc,
			.cpuAccess = BufferCpuAccess::Write,
			.memoryType = BufferMemoryType::Auto,
		}));

		std::memcpy(buffer->GetMappedPtr(), data, dataSize);

		const VkBufferCopy bufferCopy {
			0,
			0,
			dataSize
		};

		vkCmdCopyBuffer(cmd, 
			buffer.Cast<Vulkan_Buffer>()->GetVkBuffer(), staging_buffer->GetVkBuffer(),
			1, &bufferCopy);
	}

	void Vulkan_RenderAPI::ILoadDataToTexture2D(Texture2D& texture, const void* data, uint64_t dataSize) {
		auto vk_image = texture.GetImage().Cast<Vulkan_Image>();

		const VkImageMemoryBarrier2 barrier 
			= GetMemoryBarrier(*vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkDependencyInfo dependencyInfo{};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pImageMemoryBarriers = &barrier;
		dependencyInfo.imageMemoryBarrierCount = 1;

		vkCmdPipelineBarrier2(GetCurrentCommandBuffer(), &dependencyInfo);

		const auto &buffer = staging_buffers.emplace_back(ge::mem::CreateScope<Vulkan_Buffer>(BufferSpec{
			.elementSize = static_cast<uint32_t>(dataSize),
			.elementCount = 1,
			.usageFlags = BufferUsageFlagsBits::TransferSrc,
			.cpuAccess = BufferCpuAccess::Write,
			.memoryType = BufferMemoryType::Auto,
		}));

		std::memcpy(buffer->GetMappedPtr(), data, dataSize);

		VkBufferImageCopy region{};
		region.imageExtent = {texture.GetWidth(), texture.GetHeight(), 1};
		region.imageSubresource = VkImageSubresourceLayers{
			vk_image->GetAspectFlags(),
			texture.GetSubresource().baseMipmap,
			texture.GetSubresource().baseLayer,
			1
		};

		vkCmdCopyBufferToImage(
			GetCurrentCommandBuffer(), 
			buffer->GetVkBuffer(), 
			vk_image->GetImage(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			1, 
			&region);

		vk_image->_imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		_barriersForImages.emplace(vk_image.Get());
	}
	
	void Vulkan_RenderAPI::BeginDebugLabel(std::string_view label) {
		VkDebugUtilsLabelEXT label_desc{};
		label_desc.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		label_desc.pLabelName = label.data();
		label_desc.color[0] = 0.8f;
		label_desc.color[1] = 0.2f;
		label_desc.color[2] = 0.6f;
		label_desc.color[3] = 1.f;
		vkCmdBeginDebugUtilsLabelEXT(GetCurrentCommandBuffer(), &label_desc);
	}

	void Vulkan_RenderAPI::EndDebugLabel() {
		vkCmdEndDebugUtilsLabelEXT(GetCurrentCommandBuffer());
	}

	void Vulkan_RenderAPI::Barrier() {
		GEVector<VkImageMemoryBarrier2> imageBarriers;
		GEVector<VkBufferMemoryBarrier2> bufferBarriers;
		imageBarriers.reserve(_barriersForImages.size());
		bufferBarriers.reserve(_barriersForBuffers.size());

		for (auto* image : _barriersForImages) {
			const auto newLayout = utility::Vulkan_OptimalImageLayout(image->GetSpecRef().usageFlags);

			imageBarriers.emplace_back(
				GetMemoryBarrier(
						*image, 
						newLayout
					)
			);
		}

		// for (const auto* buffer : _barriersForBuffers) {
		// 	bufferBarriers.emplace_back(
		// 		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
		// 		nullptr,
		// 		utility::Vulkan_GetPipelineStageFlagsFromLayout({}),
		// 		utility::Vulkan_GetAccessFlagsFromLayout({}),
		// 		utility::Vulkan_GetPipelineStageFlagsFromLayout({}),
		// 		utility::Vulkan_GetAccessFlagsFromLayout({}),
		// 		VK_QUEUE_FAMILY_IGNORED,
		// 		VK_QUEUE_FAMILY_IGNORED,
		// 		buffer->GetVkBuffer(),
		// 		0,
		// 		VK_WHOLE_SIZE
		// 	);
		// }

		const VkDependencyInfo dependencyInfo{
			VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			nullptr,
			{},
			0,
			nullptr,
			static_cast<uint32_t>(bufferBarriers.size()),
			bufferBarriers.data(),
			static_cast<uint32_t>(imageBarriers.size()),
			imageBarriers.data(),
		};

		vkCmdPipelineBarrier2(GetCurrentCommandBuffer(), &dependencyInfo);

		_barriersForImages.clear();
		_barriersForBuffers.clear();
	}

	void Vulkan_RenderAPI::ICopyBufferToBuffer(
		const ge::mem::Ref<Buffer>& src, const ge::mem::Ref<Buffer>& dst, 
		uint32_t size, uint32_t srcOffset, uint32_t dstOffset
	) {

		// const VkDependencyInfo dependencyInfo{
		// 	VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		// 	nullptr,
		// 	{},
		// 	0,
		// 	nullptr,
		// 	static_cast<uint32_t>(bufferBarriers.size()),
		// 	bufferBarriers.data(),
		// 	nullptr,
		// 	nullptr
		// };

		// vkCmdPipelineBarrier2(GetCurrentCommandBuffer(), &dependencyInfo);

		const auto cmd = GetCurrentCommandBuffer();
		auto &vk_src = *src.Cast<Vulkan_Buffer>(); 
		auto &vk_dst = *dst.Cast<Vulkan_Buffer>(); 

		const VkBufferCopy bufferCopy {
			srcOffset,
			dstOffset,
			size
		};

		vkCmdCopyBuffer(cmd, 
			vk_src.GetVkBuffer(), vk_dst.GetVkBuffer(),
			1, &bufferCopy);

		// _barriersForBuffers.emplace(&vk_src);
		// _barriersForBuffers.emplace(&vk_dst);
	}

	void Vulkan_RenderAPI::ICopyBufferToImage(
		const ge::mem::Ref<Buffer>& src, const ge::mem::Ref<Image>& dst, 
		const ImageSubresourceLayers& imageSubresource, glm::uvec3 extent, uint32_t srcOffset, glm::uvec3 dstOffset
	) {

		const auto cmd = GetCurrentCommandBuffer();
		auto &vk_src = *src.Cast<Vulkan_Buffer>(); 
		auto &vk_dst = *dst.Cast<Vulkan_Image>(); 

		const VkImageMemoryBarrier2 barrier 
			= GetMemoryBarrier(vk_dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkDependencyInfo dependencyInfo{};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pImageMemoryBarriers = &barrier;
		dependencyInfo.imageMemoryBarrierCount = 1;

		vk_dst._imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		vkCmdPipelineBarrier2(GetCurrentCommandBuffer(), &dependencyInfo);

		const VkBufferImageCopy bufferImageCopy {
			srcOffset,
			0,
			0,
			VkImageSubresourceLayers{vk_dst.GetAspectFlags(), imageSubresource.mipLevel, 
				imageSubresource.baseArrayLayer, imageSubresource.layerCount},
			{static_cast<int32_t>(dstOffset.x), static_cast<int32_t>(dstOffset.y), static_cast<int32_t>(dstOffset.z)},
			{extent.x, extent.y, extent.z}
		};

		vkCmdCopyBufferToImage(cmd, 
			vk_src.GetVkBuffer(), 
			vk_dst.GetImage(), 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			1, 
			&bufferImageCopy
		);

		_barriersForImages.emplace(&vk_dst);
	}

	void Vulkan_RenderAPI::ICopyImageToBuffer(
		const ge::mem::Ref<Image>& src, const ImageSubresourceLayers& imageSubresource, 
		const ge::mem::Ref<Buffer>& dst, glm::uvec3 extent, glm::uvec3 srcOffset, uint32_t dstOffset
	) {
		const auto cmd = GetCurrentCommandBuffer();
		auto &vk_src = *src.Cast<Vulkan_Image>(); 
		auto &vk_dst = *dst.Cast<Vulkan_Buffer>(); 

		const VkImageMemoryBarrier2 barrier 
			= GetMemoryBarrier(vk_src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		VkDependencyInfo dependencyInfo{};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pImageMemoryBarriers = &barrier;
		dependencyInfo.imageMemoryBarrierCount = 1;

		vk_src._imageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		vkCmdPipelineBarrier2(GetCurrentCommandBuffer(), &dependencyInfo);

		const VkBufferImageCopy bufferImageCopy {
			dstOffset,
			0,
			0,
			VkImageSubresourceLayers{vk_src.GetAspectFlags(), imageSubresource.mipLevel, 
				imageSubresource.baseArrayLayer, imageSubresource.layerCount},
			{static_cast<int32_t>(srcOffset.x), static_cast<int32_t>(srcOffset.y), static_cast<int32_t>(srcOffset.z)},
			{extent.x, extent.y, extent.z}
		};

		vkCmdCopyImageToBuffer(cmd, 
			vk_src.GetImage(), 
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
			vk_dst.GetVkBuffer(), 
			1, 
			&bufferImageCopy
		);

		_barriersForImages.emplace(&vk_src);
	}

	void Vulkan_RenderAPI::ICopyImageToImage(
		const ge::mem::Ref<Image>& src, const ImageSubresourceLayers& srcSubresource, 
		const ge::mem::Ref<Image>& dst, const ImageSubresourceLayers& dstSubresource, 
		glm::uvec3 extent, glm::uvec3 srcOffset, glm::uvec3 dstOffset
	) {
		const auto cmd = GetCurrentCommandBuffer();
		auto &vk_src = *src.Cast<Vulkan_Image>(); 
		auto &vk_dst = *dst.Cast<Vulkan_Image>(); 

		const VkImageMemoryBarrier2 barrier 
			= GetMemoryBarrier(vk_src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		VkDependencyInfo dependencyInfo{};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pImageMemoryBarriers = &barrier;
		dependencyInfo.imageMemoryBarrierCount = 1;

		vk_src._imageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		vkCmdPipelineBarrier2(GetCurrentCommandBuffer(), &dependencyInfo);

		const VkImageCopy imageCopy {
			VkImageSubresourceLayers{vk_src.GetAspectFlags(), srcSubresource.mipLevel, 
				srcSubresource.baseArrayLayer, srcSubresource.layerCount},
			{static_cast<int32_t>(srcOffset.x), static_cast<int32_t>(srcOffset.y), static_cast<int32_t>(srcOffset.z)},
			VkImageSubresourceLayers{vk_dst.GetAspectFlags(), dstSubresource.mipLevel, 
				dstSubresource.baseArrayLayer, dstSubresource.layerCount},
			{static_cast<int32_t>(dstOffset.x), static_cast<int32_t>(dstOffset.y), static_cast<int32_t>(dstOffset.z)},
			{extent.x, extent.y, extent.z}
		};

		vkCmdCopyImage(cmd, 
			vk_src.GetImage(), 
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
			vk_dst.GetImage(), 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			1, 
			&imageCopy
		);

		_barriersForImages.emplace(&vk_dst);
		_barriersForImages.emplace(&vk_src);
	}
}