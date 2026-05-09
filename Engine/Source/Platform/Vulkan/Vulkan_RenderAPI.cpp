#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Core/Window.h"
#include "GlassEngine/Memory/Memory.h"
#include "GlassEngine/Renderer/Buffer.h"
#include "GlassEngine/Renderer/Renderer.h"
#include "GlassEngine/Renderer/Swapchain.h"
#include "GlassEngine/Renderer/Types.h"
#include "GlassEngine/Utilities/Counter.h"
#include "GlassEngine/Utilities/Logger.h"
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
#include <imgui_impl_vulkan.h>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ge::renderer {
	namespace utility {
		constexpr VkPipelineStageFlags2 Vulkan_GetPipelineStageFlagsFromLayout(VkImageLayout layout) noexcept {
			switch (layout) {
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				return VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				return VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;

			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
				return VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
				return VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT 
				| VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

			case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
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
					return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_SHADER_READ_BIT;

				case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
					return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

				case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
					return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

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
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		
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

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			// TODO (dnm): 
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT * (i != 1);

			vkCreateFence(VK_RENDER_CONTEXT->GetDevice(), 
					&fenceInfo, VK_ALLOCATOR_CALLBACKS, &_frames[i].inFlightFence);

			// vkCreateSemaphore(VK_RENDER_CONTEXT->GetDevice(), 
			// 		&semaphoreInfo, VK_ALLOCATOR_CALLBACKS, &_frames[i].imageAvailableSemaphore);
		}

		BeginCommandBuffer();

		auto& window = Engine::Get().GetApplicationWindow();
		auto &swapchain = static_cast<Vulkan_Swapchain &>(window.GetSwapchain());

		std::vector<VkImageMemoryBarrier> memBarrier;
		memBarrier.reserve(swapchain.GetImages().size());
		for (const auto image : swapchain.GetImages()) {
			memBarrier.emplace_back(
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, // sType
				nullptr, // pNext
				VkAccessFlags{}, // srcAccessMask
				VkAccessFlags{}, // dstAccessMask
				VK_IMAGE_LAYOUT_UNDEFINED, // oldLayout
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // newLayout
				VK_QUEUE_FAMILY_IGNORED, // srcQueueFamilyIndex
				VK_QUEUE_FAMILY_IGNORED, // dstQueueFamilyIndex
				image, // image
				VkImageSubresourceRange{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				} // subresourceRange
			);
		}

		vkCmdPipelineBarrier(
			GetCurrentCommandBuffer(),
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0,
			0, nullptr,
			0, nullptr,
			memBarrier.size(), memBarrier.data()
		);
	}

	Vulkan_RenderAPI::~Vulkan_RenderAPI()
	{
		for (uint32_t i = 0; i < Renderer3D::MaxFramesInFlight; i++) {
			vkFreeCommandBuffers(VK_RENDER_CONTEXT->GetDevice(), _frames[i].commandPool, 1, &_frames[i].commandBuffer);
			vkDestroyCommandPool(VK_RENDER_CONTEXT->GetDevice(), _frames[i].commandPool, VK_ALLOCATOR_CALLBACKS);
			vkDestroyFence(VK_RENDER_CONTEXT->GetDevice(), _frames[i].inFlightFence, VK_ALLOCATOR_CALLBACKS);
		}
	}

	void Vulkan_RenderAPI::PushConstant(const void *ptr, uint16_t size, uint16_t offset) {
		VK_RENDER_CONTEXT->GetDescriptorManager().PushConstant(GetCurrentCommandBuffer(), ptr, size, offset);
	}

	void Vulkan_RenderAPI::BeginCommandBuffer() {
		const auto frameIndex = Renderer3D::GetFrameIndex();
		auto& frame = _frames[frameIndex];
		frame.isRecordState = true;

		vkWaitForFences(VK_RENDER_CONTEXT->GetDevice(), 1, &frame.inFlightFence, VK_TRUE, UINT64_MAX);
		vkResetFences(VK_RENDER_CONTEXT->GetDevice(), 1, &frame.inFlightFence);

		for (auto& textureID : _imguiTexturePendingDeleteList)
			ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)textureID);
		_imguiTexturePendingDeleteList.clear();
		frame.renderObjects.clear();
		frame.stagingBuffers.clear();

		constexpr VkCommandBufferBeginInfo beginInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr
		};

		vkResetCommandPool(VK_RENDER_CONTEXT->GetDevice(), frame.commandPool, 0);
		vkBeginCommandBuffer(frame.commandBuffer, &beginInfo);

		VK_RENDER_CONTEXT->GetDescriptorManager().BindDescriptors(frame.commandBuffer);
	}

	void Vulkan_RenderAPI::EndCommandBuffer() {
		auto &frame = GetCurrentFrame();

		frame.isRecordState = false;
		if (frame.swapchainImageUsed) {
			auto& window = Engine::Get().GetApplicationWindow();
			auto &swapchain = static_cast<Vulkan_Swapchain &>(window.GetSwapchain());
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = {};
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = swapchain.GetCurrentImage();
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
			frame.swapchainImageUsed = false;
		}

		vkEndCommandBuffer(frame.commandBuffer);

		Renderer3D::EndFrame();
		_renderStats.Reset();
	}

	void Vulkan_RenderAPI::Draw(ge::mem::Ref<Pipeline>& pipeline, uint32_t vertexCount, uint32_t instanceCount, ge::mem::Ref<Buffer> vertexBuffer, uint32_t firstVertex, uint32_t firstInstance)
	{
		if (vertexBuffer) {
			auto vulkanVertexBuffer = vertexBuffer.Cast<Vulkan_Buffer>()->GetVkBuffer();
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(GetCurrentCommandBuffer(), 0, 1, &vulkanVertexBuffer, offsets);
		}

		auto vulkanPipeline = pipeline.Cast<Vulkan_Pipeline>()->GetPipeline();
		vkCmdBindPipeline(GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline);
		vkCmdDraw(GetCurrentCommandBuffer(), vertexCount, instanceCount, firstVertex, firstInstance);
		_renderStats.drawCalls++;

		TrackObject(pipeline);
		TrackObject(vertexBuffer);
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

		TrackObject(pipeline);
		TrackObject(vertexBuffer);
		TrackObject(indexBuffer);
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
				PushConstant(&materialAsset->GetMaterialData(), sizeof(MaterialData), (sizeof(glm::mat4) + sizeof(MaterialBindlessData)));
			}

			vkCmdDrawIndexed(GetCurrentCommandBuffer(), submesh.indexCount, 1, submesh.indexOffset, 0, 0);
			_renderStats.drawCalls++;
		}

		TrackObject(pipeline);
	}

	void Vulkan_RenderAPI::BeginCopyPass() {

	}

	void Vulkan_RenderAPI::EndCopyPass() {
		Barrier();
	}

	VkCommandBuffer Vulkan_RenderAPI::GetCurrentCommandBuffer() {
		auto &frame = GetCurrentFrame();
		if (frame.isRecordState == false)
			BeginCommandBuffer();
		return frame.commandBuffer;
	}

	void Vulkan_RenderAPI::BeginRenderPass(const BeginRenderPassSpec& spec) {
		// Barrier();

		VkCommandBuffer cmd = GetCurrentCommandBuffer();
		
		auto& window = Engine::Get().GetApplicationWindow();

		GEVector<VkRenderingAttachmentInfo> colorAttachments;
		colorAttachments.reserve(spec.colorAttachments.size());
		colorAttachments.reserve(spec.colorAttachments.size());

		GEVector<VkImageMemoryBarrier2> memBarriers;
		memBarriers.reserve(spec.colorAttachments.size() + 1);
		memBarriers.reserve(spec.colorAttachments.size() + 1);

		for (auto& attachment : spec.colorAttachments) {
			auto vk_image = attachment.image.Cast<Vulkan_Image>();
			auto& swapchain = static_cast<Vulkan_Swapchain&>(window.GetSwapchain());

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

			if (!attachment.isSwapchainImage) {
				TrackObject(attachment.image);
			} 
			else if (GetCurrentFrame().swapchainImageUsed == false) {
				memBarriers.emplace_back(
					VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, // sType
					nullptr, // pNext
					VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, // srcStageMask
					// VkPipelineStageFlags2{}, // srcStageMask
					VkAccessFlags2{}, // srcAccessMask
					VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask
					VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, // dstAccessMask
					// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // oldLayout
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // oldLayout
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // newLayout
					VK_QUEUE_FAMILY_IGNORED, // srcQueueFamilyIndex
					VK_QUEUE_FAMILY_IGNORED, // dstQueueFamilyIndex
					swapchain.GetCurrentImage(), // image
					VkImageSubresourceRange{
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.baseMipLevel = 0,
						.levelCount = 1,
						.baseArrayLayer = 0,
						.layerCount = 1
					} // subresourceRange
				);
				GetCurrentFrame().swapchainImageUsed = true;
			} 

			colorAttachments.emplace_back(
				VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				nullptr,
				attachment.isSwapchainImage ? 
					swapchain.GetCurrentImageView()
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
		if (vk_depthStencilImage) (TrackObject(vk_depthStencilImage));

		const auto has_depth = vk_depthStencilImage != nullptr;
		const auto has_stencil = has_depth ? 
			utility::IsDepthStencilFormat(vk_depthStencilImage->GetSpecRef().imageFormat) : false;

		const auto newDepthLayout = has_depth ? 
			utility::Vulkan_OptimalImageLayout(vk_depthStencilImage->GetSpecRef().usageFlags, spec.depthWrite, spec.stencilWrite) 
			: VkImageLayout{};
		if (has_depth && vk_depthStencilImage->GetImageLayout() != newDepthLayout) {
			memBarriers.emplace_back(
				GetMemoryBarrier(
					*vk_depthStencilImage, 
					newDepthLayout
				)
			);
		}
		if (has_depth) _barriersForImages.emplace(vk_depthStencilImage.Get());

		const VkRenderingAttachmentInfo depthStencilAttachment
		 	= !has_depth ? VkRenderingAttachmentInfo{} : VkRenderingAttachmentInfo{
			VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			nullptr,
			vk_depthStencilImage->CreateGetImageView(spec.depthStencilAttachment.subresource),
			newDepthLayout,
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
			TrackObject(buffer);
			return;
		}
		
		const auto &staging_buffer = GetCurrentFrame().stagingBuffers.emplace_back(ge::mem::CreateScope<Vulkan_Buffer>(BufferSpec{
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
		TrackObject(buffer);
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

		const auto &buffer = GetCurrentFrame().stagingBuffers.emplace_back(ge::mem::CreateScope<Vulkan_Buffer>(BufferSpec{
			.elementSize = static_cast<uint32_t>(dataSize),
			.elementCount = 1,
			.usageFlags = BufferUsageFlagsBits::TransferSrc,
			.cpuAccess = BufferCpuAccess::Write,
			.memoryType = BufferMemoryType::Auto,
		}));

		std::memcpy(buffer->GetMappedPtr(), data, dataSize);

		const auto pixelSize = utility::GetPixelSize(texture.GetImageFormat());
		uint32_t bufferOffset{}; 
		for (const auto i : Counter(texture.GetSubresource().mipmapCount)) {
			VkBufferImageCopy region{};
			region.imageExtent = {texture.GetWidth() >> i, texture.GetHeight() >> i, 1};
			region.imageSubresource = VkImageSubresourceLayers{
				vk_image->GetAspectFlags(),
				static_cast<uint32_t>(i),
				texture.GetSubresource().baseLayer,
				1
			};
			region.bufferOffset = bufferOffset;
			bufferOffset += region.imageExtent.width * region.imageExtent.height * pixelSize;
	
			vkCmdCopyBufferToImage(
				GetCurrentCommandBuffer(), 
				buffer->GetVkBuffer(), 
				vk_image->GetImage(),
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
				1, 
				&region);

			if (dataSize < bufferOffset / 4)
				break;
		}

		vk_image->_imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		_barriersForImages.emplace(vk_image.Get());
		TrackObject(vk_image);
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
		TrackObject(src);
		TrackObject(dst);

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
		TrackObject(src);
		TrackObject(dst);
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
		TrackObject(src);
		TrackObject(dst);
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
		TrackObject(src);
		TrackObject(dst);
	}

	ImTextureID Vulkan_RenderAPI::GetImGuiTexture(const GEString& name, ge::mem::Ref<Image>& image, ImageSubresource subresource, ge::mem::Ref<Sampler> sampler) {
		if (_imguiTextureCache.contains(name) && _imguiTextureCache[name] != 0 && image != nullptr) {
			_imguiTexturePendingDeleteList.push_back(_imguiTextureCache[name]);
		}
		else if (image == nullptr) {
			return _imguiTextureCache[name];
		}

		_imguiTextureCache[name] = (ImTextureID)ImGui_ImplVulkan_AddTexture(
			sampler.Cast<Vulkan_Sampler>()->GetSampler(),
			image.Cast<Vulkan_Image>()->CreateGetImageView(subresource),
			utility::Vulkan_OptimalImageLayout(image->GetSpecRef().usageFlags));
		TrackObject(image);
		return _imguiTextureCache[name];
	}
}