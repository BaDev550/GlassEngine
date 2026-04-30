#include "gepch.h"
#include "GlassEngine/Renderer/Renderer.h"
#include "Vulkan_ImGuiLayer.h"
#include "Vulkan_Swapchain.h"
#include "Vulkan_RenderAPI.h"
#include "Vulkan_RenderContext.h"

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

		VkExtent2D extent = VkExtent2D({ swapchain->GetExtent().x, swapchain->GetExtent().y });

		VkRenderingAttachmentInfo colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageView = swapchain->GetImageViews()[Engine::Get().GetApplicationWindow().GetImageIndex()];
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = { {0, 0}, extent };
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;

		renderer::Renderer3D::GetRenderAPI()->BeginDebugLabel("IMGUI_PASS");
		vkCmdBeginRendering(cmd, &renderingInfo);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
		vkCmdEndRendering(cmd);
		renderer::Renderer3D::GetRenderAPI()->EndDebugLabel();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}
}