#include "GlassEngine/Renderer/RenderAPI.h"
#include "GlassEngine/Renderer/Types.h"
#include "gepch.h"
#include "GlassEngine/Renderer/Renderer.h"
#include "Vulkan_ImGuiLayer.h"
#include "Vulkan_Swapchain.h"
#include "Vulkan_RenderAPI.h"
#include "Vulkan_RenderContext.h"

#include <glm/ext/vector_float4.hpp>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <imgui.h>

namespace ge {
	Vulkan_ImGuiLayer::Vulkan_ImGuiLayer()
	{
		auto& window = Engine::Get().GetApplicationWindow();
		auto context = CastChecked<renderer::Vulkan_RenderContext>(&window.GetRenderContext());
		auto swapchain = CastChecked<renderer::Vulkan_Swapchain>(&window.GetSwapchain());

		GEVector<VkDescriptorPoolSize> poolSize;
		poolSize.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,        1000 });
		poolSize.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,         1000 });
		poolSize.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 });
		poolSize.push_back({ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,         1000 });
		poolSize.push_back({ VK_DESCRIPTOR_TYPE_SAMPLER,   1000 });
		
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
		descriptorPoolCreateInfo.pPoolSizes = poolSize.data();
		descriptorPoolCreateInfo.maxSets = 1000;
		descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		vkCreateDescriptorPool(context->GetDevice(), &descriptorPoolCreateInfo, VK_ALLOCATOR_CALLBACKS, &_descriptorPool);

		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		auto& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGui_ImplGlfw_InitForVulkan(window.GetHandle(), true);
		ImGui_ImplVulkan_PipelineInfo pipelineInfo{};
		VkFormat swapchainColorImageFormat = swapchain->GetSwapchainFormat();
		pipelineInfo.RenderPass = nullptr;
		pipelineInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		pipelineInfo.PipelineRenderingCreateInfo.sType = { VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
		pipelineInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		pipelineInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchainColorImageFormat;
		pipelineInfo.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
		
		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = context->GetInstance();
		initInfo.PhysicalDevice = context->GetPhysicalDevice();
		initInfo.Device = context->GetDevice();
		initInfo.Queue = context->GetGraphicsQueue();
		initInfo.DescriptorPool = _descriptorPool;
		initInfo.MinImageCount = swapchain->GetImages().size();
		initInfo.Allocator = VK_ALLOCATOR_CALLBACKS;
		initInfo.PipelineCache = context->GetPipelineCache();
		initInfo.ImageCount = swapchain->GetImages().size();
		initInfo.UseDynamicRendering = true;
		initInfo.PipelineInfoMain = pipelineInfo;
		ImGui_ImplVulkan_Init(&initInfo);
		ImGui_ImplVulkan_CreateMainPipeline(&pipelineInfo);
	}

	Vulkan_ImGuiLayer::~Vulkan_ImGuiLayer()
	{
		auto& window = Engine::Get().GetApplicationWindow();
		auto context = CastChecked<renderer::Vulkan_RenderContext>(&window.GetRenderContext());
		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplVulkan_Shutdown();
		vkDestroyDescriptorPool(context->GetDevice(), _descriptorPool, VK_ALLOCATOR_CALLBACKS);
		ImGui::DestroyContext();
	}

	void Vulkan_ImGuiLayer::OnAttach()
	{
	}
	void Vulkan_ImGuiLayer::OnDetach()
	{
	}

	void Vulkan_ImGuiLayer::Begin()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Vulkan_ImGuiLayer::End()
	{
		ImGui::Render();
		auto& window = Engine::Get().GetApplicationWindow();
		auto& context = window.GetRenderContext();
		auto swapchain = CastChecked<renderer::Vulkan_Swapchain>(&window.GetSwapchain());
		auto renderAPI = renderer::Renderer3D::GetRenderAPI().Cast<renderer::Vulkan_RenderAPI>();

		auto cmd = renderAPI->GetCurrentCommandBuffer();
		
		renderer::Attachment colorAttachment{};
		colorAttachment.clearValue = glm::vec4{0,0,0, 0};
		colorAttachment.isSwapchainImage = true;
		colorAttachment.loadOp = renderer::AttachmentLoadOp::Clear;
		colorAttachment.storeOp = renderer::AttachmentStoreOp::Store;

		renderer::BeginRenderPassSpec beginDesc{};
		beginDesc.colorAttachments = std::span(&colorAttachment, 1);
		beginDesc.extent = swapchain->GetExtent();

		renderer::Renderer3D::GetRenderAPI()->BeginDebugLabel("IMGUI_PASS");
		renderer::Renderer3D::GetRenderAPI()->BeginRenderPass(beginDesc);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
		renderer::Renderer3D::GetRenderAPI()->EndRenderPass();
		renderer::Renderer3D::GetRenderAPI()->EndDebugLabel();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}
}