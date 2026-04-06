#pragma once
#include "GlassEngine/Renderer/Shader.h"
#include "Vulkan_RenderContext.h"
#include <map>
#include "Vulkan_Types.h"

namespace ge::renderer {
	struct CompiledData {
		std::map<uint32_t, std::map<uint32_t, DescriptorInfo>> ReflectData;
		GEVector<VkVertexInputAttributeDescription> AttribDescriptions;
		VkVertexInputBindingDescription BindingDescription;
	};

	class Vulkan_Shader : public Shader {
	public:
		Vulkan_Shader(const GEString& vertexPath, const GEString& fragmentPath);
		~Vulkan_Shader();
		virtual void Compile() override;

		VkShaderModule GetVertexModule() const { return _vertexModule; }
		VkShaderModule GetFragmentModule() const { return _fragmentModule; }

		[[nodiscard]] const CompiledData& GetCompiledData() const { return _compiledData; }
		[[nodiscard]] const std::map<uint32_t, VkDescriptorSetLayout>& GetDescriptorLayouts() const { return _descriptorLayouts; }
		[[nodiscard]] const std::map<uint32_t, std::map<uint32_t, DescriptorInfo>>& GetReflectData() const { return _compiledData.ReflectData; }
	private:
		void CreateShaderModule(const GEVector<char>& code, VkShaderModule& shaderModule);
		GEString _vertexPath, _fragmentPath;
		CompiledData _compiledData;
		VkShaderModule _vertexModule;
		VkShaderModule _fragmentModule;
		std::map<uint32_t, VkDescriptorSetLayout> _descriptorLayouts;
	};
}