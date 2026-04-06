#pragma once
#include "Vulkan_Shader.h"
#include <string>
#include <filesystem>

namespace ge::renderer {
	static constexpr uint32_t GetFormatSize(VkFormat format) {
		switch (format)
		{
		case VK_FORMAT_UNDEFINED: return 0;
		case VK_FORMAT_R8G8B8A8_UNORM: return 4;
		case VK_FORMAT_R8G8B8A8_SRGB: return 4;
		case VK_FORMAT_R16_SFLOAT: return 2;
		case VK_FORMAT_R16G16_SFLOAT: return 4;
		case VK_FORMAT_R16G16B16_SFLOAT: return 8;
		case VK_FORMAT_R16G16B16A16_SFLOAT: return 16;
		case VK_FORMAT_R32_SFLOAT: return 4;
		case VK_FORMAT_R32G32_SFLOAT: return 8;
		case VK_FORMAT_R32G32B32_SFLOAT: return 12;
		case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;
		}
	} // TODO (0x): move this into an own folder

	static constexpr VkDescriptorType ShaderReflectionTypeToVulkanType(const ShaderDataType& type) {
		switch (type)
		{
		case ShaderDataType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case ShaderDataType::Sampler2D: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case ShaderDataType::SamplerCube: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		default: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}
	}

    class ShaderCompiler { // TEMP Class
    public:
        static GEVector<char> CompileShaderFileToSpirv(const std::filesystem::path& path, CompiledData& shaderInfo, bool optimize = false);
        static void CollectReflectionData(CompiledData& shaderInfo, const void* code, size_t sizeInBytes);
    };
}