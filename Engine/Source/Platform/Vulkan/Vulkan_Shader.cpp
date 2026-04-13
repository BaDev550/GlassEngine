#include "gepch.h"
#include "Vulkan_Shader.h"

namespace ge::renderer {
	Vulkan_Shader::Vulkan_Shader(const GEString& vertexPath, const GEString& fragmentPath) : _vertexPath(vertexPath), _fragmentPath(fragmentPath) {}
	Vulkan_Shader::~Vulkan_Shader() {
		for (auto& [set, layout] : _descriptorLayouts)
			vkDestroyDescriptorSetLayout(VK_RENDER_CONTEXT->GetDevice(), layout, VK_ALLOCATOR_CALLBACKS);
		vkDestroyShaderModule(VK_RENDER_CONTEXT->GetDevice(), _vertexModule, VK_ALLOCATOR_CALLBACKS);
		vkDestroyShaderModule(VK_RENDER_CONTEXT->GetDevice(), _fragmentModule, VK_ALLOCATOR_CALLBACKS);
	}

	void Vulkan_Shader::Compile() {
		//GEVector<char> vertShaderCode = ShaderCompiler::CompileShaderFileToSpirv(_vertexPath, _compiledData); // TODO (dnm): move the compiler code and class to slang
		//GEVector<char> fragShaderCode = ShaderCompiler::CompileShaderFileToSpirv(_fragmentPath, _compiledData);
		//CreateShaderModule(vertShaderCode, _vertexModule);
		//CreateShaderModule(fragShaderCode, _fragmentModule);

		//for (auto const& [set, bindings] : _compiledData.ReflectData) {
		//	GEVector<VkDescriptorSetLayoutBinding> layoutBindings;
		//	for (auto const& [binding, info] : bindings) {
		//		VkDescriptorType vkType = utility::ShaderReflectionTypeToVulkanType(info.type);
		//		VkDescriptorSetLayoutBinding layoutBinding{};
		//		layoutBinding.binding = binding;
		//		layoutBinding.descriptorType = vkType,
		//			layoutBinding.descriptorCount = info.count,
		//			layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		//		layoutBindings.push_back(layoutBinding);
		//	}
		//	VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
		//	layoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
		//	layoutCreateInfo.pBindings = layoutBindings.data();
		//	vkCreateDescriptorSetLayout(VK_RENDER_CONTEXT->GetDevice(), &layoutCreateInfo, VK_ALLOCATOR_CALLBACKS, &_descriptorLayouts[set]);
		//	_shaderStage = ShaderStat::LoadedCompiled;
		//}
	}

	void Vulkan_Shader::CreateShaderModule(const GEVector<char>& code, VkShaderModule& shaderModule)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		vkCreateShaderModule(VK_RENDER_CONTEXT->GetDevice(), &createInfo, VK_ALLOCATOR_CALLBACKS, &shaderModule);
	}
}