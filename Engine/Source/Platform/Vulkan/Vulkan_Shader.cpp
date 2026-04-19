#include "gepch.h"
#include "Vulkan_Shader.h"

namespace ge::renderer {
	Vulkan_Shader::Vulkan_Shader(std::string_view shaderName, ShaderData&& shaderData)
		: renderer::Shader(shaderName, std::move(shaderData)) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.codeSize = GetSpirvByteCode().size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(GetSpirvByteCode().data());
		vkCreateShaderModule(VK_RENDER_CONTEXT->GetDevice(), &createInfo, VK_ALLOCATOR_CALLBACKS, &_shaderModule);
	}
}