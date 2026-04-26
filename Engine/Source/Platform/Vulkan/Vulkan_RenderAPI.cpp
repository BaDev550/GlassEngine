#include "GlassEngine/Core/Core.h"
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
	
		constexpr VkAccessFlags2 Vulkan_GetAccessFlagsFromLayout(VkImageLayout layout) noexcept {
			switch (layout) {
				case VK_IMAGE_LAYOUT_GENERAL: 
					return VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
					return VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
					return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
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
		barrier.oldLayout = frameIndex <= swapchain.GetImages().size() ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
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
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
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
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			//swapchain->ReCreateSwapchain();
		}
		_renderStats.drawCalls = 0;
		_frameStarted = false;
	}

	void Vulkan_RenderAPI::Draw(uint32_t vertexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, uint32_t firstVertex, uint32_t firstInstance)
	{
		auto vulkanVertexBuffer = vertexBuffer.Cast<Vulkan_Buffer>()->GetVkBuffer();
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(GetCurrentCommandBuffer(), 0, 1, &vulkanVertexBuffer, offsets);
		vkCmdDraw(GetCurrentCommandBuffer(), vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void Vulkan_RenderAPI::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, ge::mem::Ref<Buffer> indexBuffer, uint32_t firstIndex, uint32_t firstInstance, int32_t vertexOffset)
	{
		auto vulkanVertexBuffer = vertexBuffer.Cast<Vulkan_Buffer>()->GetVkBuffer();
		auto vulkanIndexBuffer = indexBuffer.Cast<Vulkan_Buffer>()->GetVkBuffer();
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(GetCurrentCommandBuffer(), 0, 1, &vulkanVertexBuffer, offsets);
		vkCmdBindIndexBuffer(GetCurrentCommandBuffer(), vulkanIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(GetCurrentCommandBuffer(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void Vulkan_RenderAPI::DrawStaticMesh(ge::mem::Ref<Pipeline>& pipeline, ge::mem::Ref<StaticMesh>& mesh, uint32_t lodIndex, ge::mem::Ref<MaterialTable> materialTable, const glm::mat4& transform)
	{
		const auto& currentLOD = mesh->GetLODs()[lodIndex];
		auto& materialAssets = materialTable ? materialTable->GetMaterials() : mesh->GetMaterialTable()->GetMaterials();
		auto vulkanPipeline = pipeline.Cast<Vulkan_Pipeline>()->GetPipeline();

		vkCmdBindPipeline(GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline);
		for (const Submesh& submesh : currentLOD.submesh) {
			auto material = materialAssets[submesh.materialIndex]->GetMaterial();
			PushConstant(&material->GetBindlessData(), sizeof(MaterialBindlessData), 0);
			DrawIndexed(submesh.indexCount, 1, mesh->GetVertexBuffer(), mesh->GetIndexBuffer(), submesh.indexOffset, 0, submesh.vertexOffset);
		}
	}

	void Vulkan_RenderAPI::BeginCopyPass() {

	}

	void Vulkan_RenderAPI::EndCopyPass() {
		TransitionImageLayouts();
	}

	VkCommandBuffer Vulkan_RenderAPI::GetCurrentCommandBuffer() {
		GE_ASSERT(_frameStarted, "Cannot get active command buffer while frame is not started");
		return _frames[Renderer3D::GetFrameIndex()].commandBuffer;
	}

	void Vulkan_RenderAPI::TransitionImageLayouts() {
		GEVector<VkImageMemoryBarrier2> barriers;
		barriers.reserve(_image_layout_transition_set.size());

		for (const auto* image : _image_layout_transition_set) {
			const auto newLayout = utility::Vulkan_OptimalImageLayout(image->GetSpecRef().usageFlags);

			barriers.emplace_back(
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				nullptr,
				utility::Vulkan_GetPipelineStageFlagsFromLayout(image->GetImageLayout()),
				utility::Vulkan_GetAccessFlagsFromLayout(image->GetImageLayout()),
				utility::Vulkan_GetPipelineStageFlagsFromLayout(newLayout),
				utility::Vulkan_GetAccessFlagsFromLayout(newLayout),
				image->GetImageLayout(),
				newLayout,
				VK_QUEUE_FAMILY_IGNORED,
				VK_QUEUE_FAMILY_IGNORED,
				image->GetImage(),
				VkImageSubresourceRange{
					.aspectMask = image->GetAspectFlags(),
					.baseMipLevel = 0,
					.levelCount = VK_REMAINING_MIP_LEVELS,
					.baseArrayLayer = 0,
					.layerCount =  VK_REMAINING_ARRAY_LAYERS
				}
			);
		}

		VkDependencyInfo dependencyInfo{};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pImageMemoryBarriers = barriers.data();
		dependencyInfo.imageMemoryBarrierCount = barriers.size();

		vkCmdPipelineBarrier2(GetCurrentCommandBuffer(), &dependencyInfo);

		_image_layout_transition_set.clear();
	}

	void Vulkan_RenderAPI::BeginRenderPass(const BeginRenderPassSpec& spec) {
		VkCommandBuffer cmd = Renderer3D::GetRenderAPI().Cast<Vulkan_RenderAPI>()->GetCurrentCommandBuffer();
		
		GEVector<VkRenderingAttachmentInfo> colorAttachments;
		colorAttachments.reserve(spec.color_attachments.size());

		for (const auto& attachment : spec.color_attachments) {
			auto vk_image = attachment.image.Cast<Vulkan_Image>();
			colorAttachments.emplace_back(
				VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				nullptr,
				vk_image->CreateGetImageView(attachment.subresource),
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VkResolveModeFlagBits{},
				VkImageView{},
				VkImageLayout{},
				utility::Vulkan_GetLoadOp(attachment.loadOp),
				utility::Vulkan_GetStoreOp(attachment.storeOp),
				VkClearValue{.color{
					attachment.clearColor.colorClear.r, 
					attachment.clearColor.colorClear.g, 
					attachment.clearColor.colorClear.b, 
					attachment.clearColor.colorClear.a
				}}
			);
		}

		auto vk_image = spec.depth_stencil.image.Cast<Vulkan_Image>();

		const VkRenderingAttachmentInfo depthStencilAttachment
		 	= vk_image ? VkRenderingAttachmentInfo{} : VkRenderingAttachmentInfo{
			VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			nullptr,
			vk_image->CreateGetImageView(spec.depth_stencil.subresource),
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VkResolveModeFlagBits{},
			VkImageView{},
			VkImageLayout{},
			utility::Vulkan_GetLoadOp(spec.depth_stencil.loadOp),
			utility::Vulkan_GetStoreOp(spec.depth_stencil.storeOp),
			VkClearValue{.depthStencil{
				spec.depth_stencil.clearColor.depthClear,
				spec.depth_stencil.clearColor.stencilClear
			}}
		};

		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = VkRect2D({ 0, 0 }, {spec.extent.x, spec.extent.y});
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
		renderingInfo.pColorAttachments = colorAttachments.data();
		renderingInfo.pDepthAttachment = &depthStencilAttachment;
		renderingInfo.pStencilAttachment = &depthStencilAttachment;
		
		VkViewport viewport{ 0, 0, (float)spec.extent.x, (float)spec.extent.y };
		viewport.minDepth = 0;
		viewport.maxDepth = 1;
		VkRect2D scissor{ {0,0}, {spec.extent.x, spec.extent.y} };

		vkCmdBeginRendering(cmd, &renderingInfo);
		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdSetScissor(cmd, 0, 1, &scissor);
	}

	void Vulkan_RenderAPI::EndRenderPass() {
		_image_layout_transition_set.clear();
	}

	void Vulkan_RenderAPI::LoadDataToBuffer(const ge::mem::Ref<Buffer>& buffer, const void* data, uint64_t dataSize) {
		
	}

	void Vulkan_RenderAPI::LoadDataToTexture2D(Texture2D& texture, const void* data, uint64_t dataSize) {
		auto vk_image = texture.GetImage().Cast<Vulkan_Image>();

		const VkImageMemoryBarrier2 barrier{
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			nullptr,
			utility::Vulkan_GetPipelineStageFlagsFromLayout(vk_image->GetImageLayout()),
			utility::Vulkan_GetAccessFlagsFromLayout(vk_image->GetImageLayout()),
			utility::Vulkan_GetPipelineStageFlagsFromLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
			utility::Vulkan_GetAccessFlagsFromLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
			vk_image->GetImageLayout(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			vk_image->GetImage(),
			VkImageSubresourceRange{
				.aspectMask = vk_image->GetAspectFlags(),
				.baseMipLevel = 0,
				.levelCount = VK_REMAINING_MIP_LEVELS,
				.baseArrayLayer = 0,
				.layerCount =  VK_REMAINING_ARRAY_LAYERS
			}
		};

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
		_image_layout_transition_set.emplace(vk_image.Get());
	}
	
	void Vulkan_RenderAPI::SetBeginDebugLabel(std::string_view label) {
		VkDebugUtilsLabelEXT label_desc{};
		label_desc.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		label_desc.pLabelName = label.data();
		label_desc.color[0] = 0.8f;
		label_desc.color[1] = 0.2f;
		label_desc.color[2] = 0.6f;
		label_desc.color[3] = 1.f;
		vkCmdBeginDebugUtilsLabelEXT(GetCurrentCommandBuffer(), &label_desc);
	}

	void Vulkan_RenderAPI::SetEndDebugLabel() {

	}
}