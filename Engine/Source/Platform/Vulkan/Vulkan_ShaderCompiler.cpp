#include "gepch.h"
#include "Vulkan_ShaderCompiler.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <shaderc/shaderc.hpp>

#define SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
#include <spirv_reflect.h>

namespace ge::renderer {
    // Forked from https://github.com/beaumanvienna/vulkan/blob/617f70e1a311c6f498ec69507dcc9d4aadb86612/engine/platform/Vulkan/VKshader.cpp
    class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface {
    public:
        ShaderIncluder(std::filesystem::path shaderDir) : _ShaderDirectory(shaderDir) {}
        shaderc_include_result* GetInclude(const char* requestedSource, shaderc_include_type type, const char* requestingSource, size_t includeDepth) override;
        void ReleaseInclude(shaderc_include_result* data) override;
        std::string ReadFile(const std::string& filepath);
        std::filesystem::path _ShaderDirectory;
    };

    namespace utils {
        shaderc_shader_kind InferShaderKind(const std::filesystem::path& path) {
            const auto ext = path.extension().string();
            if (ext == ".vert") return shaderc_vertex_shader;
            if (ext == ".frag") return shaderc_fragment_shader;
            if (ext == ".comp") return shaderc_compute_shader;
            if (ext == ".geom") return shaderc_geometry_shader;
            if (ext == ".tesc") return shaderc_tess_control_shader;
            if (ext == ".tese") return shaderc_tess_evaluation_shader;
            return shaderc_glsl_infer_from_source;
        }

        std::vector<uint32_t> LoadShaderFile(const std::filesystem::path& filePath) {
            std::ifstream file(filePath, std::ios::ate | std::ios::binary);
            const auto fileSize = file.tellg();
            std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
            file.seekg(0);
            file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
            file.close();
            return buffer;
        }

        std::string ReadFileToString(const std::filesystem::path& path) {
            std::ifstream in(path, std::ios::binary);
            GE_ASSERT(in, "Failed to open shader file: " + path.string());
            std::ostringstream ss;
            ss << in.rdbuf();
            return ss.str();
        }

        constexpr ShaderDataType GetResourceType(const SpvReflectDescriptorBinding* binding) {
            switch (binding->descriptor_type)
            {
            case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                if (binding->image.dim == SpvDimCube) {
                    return ShaderDataType::SamplerCube;
                }
                return ShaderDataType::Sampler2D;
            case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:         return ShaderDataType::UniformBuffer;
            case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return ShaderDataType::UniformBuffer;
            default: return ShaderDataType::UniformBuffer;
            }
        }
    }

    GEVector<char> ShaderCompiler::CompileShaderFileToSpirv(const std::filesystem::path& path, CompiledData& shaderInfo, bool optimize)
    {
        if (path.extension() == ".spv") {
            std::ifstream in(path, std::ios::binary);
            GE_ASSERT(in, "Failed to open .spv file: " + path.string());
            GEVector<char> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            CollectReflectionData(shaderInfo, data.data(), data.size());
            return data;
        }

        const std::string source = utils::ReadFileToString(path);
        shaderc::Compiler compiler;
        GE_ASSERT(compiler.IsValid(), "shaderc::Compiler failed to initialize");
        shaderc_shader_kind kind = utils::InferShaderKind(path);
        shaderc::CompileOptions options;

        options.SetGenerateDebugInfo();
        options.SetIncluder(ge::mem::CreateScope<ShaderIncluder>(path.parent_path()));
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_1);
        if (optimize) {
            options.SetOptimizationLevel(shaderc_optimization_level_performance);
        }
        else {
            options.SetOptimizationLevel(shaderc_optimization_level_zero);
        }
        options.SetWarningsAsErrors();

        auto precompileResult = compiler.PreprocessGlsl(source, kind, path.string().c_str(), options);
        if (precompileResult.GetCompilationStatus() != shaderc_compilation_status_success)
            GE_GRAPHICS_ERROR("VK_Shader: Could not preompile shader {0}, error message: {1}", path.string(), precompileResult.GetErrorMessage());

        auto compileResult = compiler.CompileGlslToSpv(source, kind, path.string().c_str(), options);
        if (compileResult.GetCompilationStatus() != shaderc_compilation_status_success)
            GE_GRAPHICS_ERROR("VK_Shader: Could not compile shader {0}, error message: {1}", path.string(), compileResult.GetErrorMessage());

        const auto status = compileResult.GetCompilationStatus();
        if (status != shaderc_compilation_status_success) {
            std::string err = compileResult.GetErrorMessage();
            GE_ASSERT(false, "Shader compile error (" + path.string() + "): " + err)
        }

        const uint32_t* begin = compileResult.cbegin();
        const uint32_t* end = compileResult.cend();
        const size_t word_count = static_cast<size_t>(end - begin);

        GEVector<char> bytes;
        bytes.resize(word_count * sizeof(uint32_t));
        if (word_count > 0) {
            std::memcpy(bytes.data(), reinterpret_cast<const char*>(begin), bytes.size());
        }

        CollectReflectionData(shaderInfo, bytes.data(), bytes.size());
        return bytes;
    }

    void ShaderCompiler::CollectReflectionData(CompiledData& shaderInfo, const void* code, size_t sizeInBytes)
    {
        SpvReflectShaderModule module;
        SpvReflectResult result = spvReflectCreateShaderModule(sizeInBytes, code, &module);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        uint32_t var_count = 0;
        result = spvReflectEnumerateInputVariables(&module, &var_count, NULL);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);
        std::vector<SpvReflectInterfaceVariable*> input_vars(var_count);
        result = spvReflectEnumerateInputVariables(&module, &var_count, input_vars.data());

        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        uint32_t count;
        spvReflectEnumerateDescriptorBindings(&module, &count, nullptr);
        std::vector<SpvReflectDescriptorBinding*> bindings(count);
        spvReflectEnumerateDescriptorBindings(&module, &count, bindings.data());

        for (const auto& ds : bindings) {
            shaderInfo.ReflectData[ds->set][ds->binding] = { ds->name, utils::GetResourceType(ds), ds->count };
            GE_GRAPCHICS_INFO("Found input/buffer n:{}, b:{}", ds->name, ds->binding);
        }

        if (module.shader_stage & SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) {
            shaderInfo.AttribDescriptions.clear();

            spvReflectEnumerateInputVariables(&module, &count, nullptr);
            if (count > 0) {
                std::vector<SpvReflectInterfaceVariable*> inputs(count);
                spvReflectEnumerateInputVariables(&module, &count, inputs.data());
                std::vector<SpvReflectInterfaceVariable*> filteredInputs;
                for (auto* input : inputs) {
                    if (input->location != 0xFFFFFFFF) {
                        filteredInputs.push_back(input);
                    }
                }
                std::sort(filteredInputs.begin(), filteredInputs.end(), [](SpvReflectInterfaceVariable* a, SpvReflectInterfaceVariable* b) { return a->location < b->location; });

                uint32_t stride = 0;
                for (const auto& input : filteredInputs) {
                    VkFormat format = static_cast<VkFormat>(input->format);
                    uint32_t size = GetFormatSize(format);

                    VkVertexInputAttributeDescription attrib{};
                    attrib.location = input->location;
                    attrib.format = format;
                    attrib.offset = stride;

                    shaderInfo.AttribDescriptions.emplace_back(attrib);
                    stride += size;
                }
                shaderInfo.BindingDescription.binding = 0;
                shaderInfo.BindingDescription.stride = stride;
                shaderInfo.BindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            }
        }
        spvReflectDestroyShaderModule(&module);
    }

    shaderc_include_result* ShaderIncluder::GetInclude(const char* requestedSource, shaderc_include_type type, const char* requestingSource, size_t includeDepth)
    {
        std::string msg = std::string(requestingSource);
        msg += std::to_string(type);
        msg += static_cast<char>(includeDepth);

        const std::string name = std::string(requestedSource);
        const std::string contents = ReadFile(name);

        auto container = new std::array<std::string, 2>;
        (*container)[0] = name;
        (*container)[1] = contents;

        auto data = new shaderc_include_result;

        data->user_data = container;

        data->source_name = (*container)[0].data();
        data->source_name_length = (*container)[0].size();

        data->content = (*container)[1].data();
        data->content_length = (*container)[1].size();

        return data;
    }

    void ShaderIncluder::ReleaseInclude(shaderc_include_result* data)
    {
        delete static_cast<std::array<std::string, 2>*>(data->user_data);
        delete data;
    }

    std::string ShaderIncluder::ReadFile(const std::string& filepath)
    {
        std::string sourceCode;
        std::ifstream in(_ShaderDirectory / filepath, std::ios::in | std::ios::binary);
        if (in)
        {
            in.seekg(0, std::ios::end);
            size_t size = in.tellg();
            if (size > 0)
            {
                sourceCode.resize(size);
                in.seekg(0, std::ios::beg);
                in.read(&sourceCode[0], size);
            }
            else
            {
                GE_GRAPHICS_ERROR("ShaderIncluder::ReadFile: Could not read shader file '{0}'", filepath);
            }
        }
        else
        {
            GE_GRAPHICS_ERROR("ShaderIncluder::ReadFile Could not open shader file '{0}'", filepath);
        }
        return sourceCode;
    }
}