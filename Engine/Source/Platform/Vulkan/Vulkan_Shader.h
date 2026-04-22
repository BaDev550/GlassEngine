#pragma once
#include "GlassEngine/Renderer/Shader.h"
#include "Vulkan_RenderContext.h"
#include "Vulkan_Types.h"
#include <map>

namespace ge::renderer {
	class Vulkan_Shader : public Shader {
	public:
		Vulkan_Shader(std::string_view shaderName, ShaderData&& shaderData);
		~Vulkan_Shader() {
			vkDestroyShaderModule(VK_RENDER_CONTEXT->GetDevice(), _shaderModule, VK_ALLOCATOR_CALLBACKS);
		}

		[[nodiscard]] VkShaderModule GetShaderModule() const noexcept { return _shaderModule; }
		virtual void SetDebugName(GEString name) const noexcept override;
	private:
		VkShaderModule _shaderModule;
	};
}