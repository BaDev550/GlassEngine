#pragma once
#include "GlassEngine/Layers/ImGuiLayer.h"
#include "Vulkan_RenderContext.h"

namespace ge {
	class Vulkan_ImGuiLayer : public ImGuiLayer {
	public:
		Vulkan_ImGuiLayer();
		~Vulkan_ImGuiLayer();
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void Begin() override;
		virtual void End() override;
	private:
		VkDescriptorPool _descriptorPool;
	};
}