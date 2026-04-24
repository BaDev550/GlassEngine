#include "gepch.h"
#include "Vulkan_Shader.h"

namespace ge::renderer {
	Vulkan_Shader::Vulkan_Shader(std::string_view shaderName, ShaderData&& shaderData) : renderer::Shader(shaderName, std::move(shaderData)) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = GetSpirvByteCode().size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(GetSpirvByteCode().data());
		vkCreateShaderModule(VK_RENDER_CONTEXT->GetDevice(), &createInfo, VK_ALLOCATOR_CALLBACKS, &_shaderModule);
	}

	void Vulkan_Shader::SetDebugName(GEString name) const noexcept {
		VkDebugUtilsObjectNameInfoEXT nameInfo{};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectHandle = reinterpret_cast<uint64_t>(_shaderModule);
		nameInfo.objectType = VK_OBJECT_TYPE_SHADER_MODULE;
		nameInfo.pObjectName = name.c_str();

		vkSetDebugUtilsObjectNameEXT(VK_RENDER_CONTEXT->GetDevice(), &nameInfo);
	}
}