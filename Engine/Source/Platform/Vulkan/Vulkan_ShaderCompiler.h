#pragma once
#include "Vulkan_Shader.h"
#include <string>
#include <filesystem>

namespace ge::renderer {
    class ShaderCompiler { // TEMP Class
    public:
        static GEVector<char> CompileShaderFileToSpirv(const std::filesystem::path& path, CompiledData& shaderInfo, bool optimize = false);
        static void CollectReflectionData(CompiledData& shaderInfo, const void* code, size_t sizeInBytes);
    };
}