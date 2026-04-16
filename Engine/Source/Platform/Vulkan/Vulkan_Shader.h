#pragma once
#include "GlassEngine/Renderer/Shader.h"
#include "Vulkan_RenderContext.h"
#include "Vulkan_Types.h"
#include <map>
#include "Vulkan_Types.h"

namespace ge::renderer {
	class Vulkan_Shader : public Shader {
	public:
		Vulkan_Shader(const GEString& vertexPath, const GEString& fragmentPath);
		~Vulkan_Shader();
		virtual void Compile() override;

		VkShaderModule GetVertexModule() const { return _vertexModule; }
		VkShaderModule GetFragmentModule() const { return _fragmentModule; }

		virtual ShaderReflection& GetReflectionData() override { return _compiledData; };
	private:
		void CreateShaderModule(const GEVector<char>& code, VkShaderModule& shaderModule);
		GEString _vertexPath, _fragmentPath;
		ShaderReflection _compiledData;
		VkShaderModule _vertexModule;
		VkShaderModule _fragmentModule;
		std::map<uint32_t, VkDescriptorSetLayout> _descriptorLayouts;
	};
}