#include "gepch.h"
#include "ShaderCompiler.h"
#include <GlassEngine/Utilities/Counter.h>
#include <slang.h>
#include <slang-com-ptr.h>
#include <slang-com-helper.h>

namespace ge::renderer {
    static constexpr uint32_t BindlessReadonlyImageSetIndex = 0;
    static constexpr uint32_t BindlessWritableImageSetIndex = 1;
    static constexpr uint32_t BindlessSamplerSetIndex = 2;
    static constexpr uint32_t UserResourceSetIndex = 3;

    static constexpr uint32_t InShaderUserResourceSetIndex = 0;
    static constexpr uint32_t InShaderBindlessReadonlyImageSetIndex = 1;
    static constexpr uint32_t InShaderBindlessWritableImageSetIndex = 2;
    static constexpr uint32_t InShaderBindlessSamplerSetIndex = 3;

    static constexpr uint32_t SpirvTargetIndex = 0;
    static constexpr uint32_t DxilTargetIndex = 1;

    static constexpr ShaderResourceType SlangToGE(const slang::BindingType type) {
        switch (type) {
            case slang::BindingType::Sampler: return ShaderResourceType::Sampler;
            case slang::BindingType::Texture: return ShaderResourceType::ReadonlyImage;
            case slang::BindingType::ConstantBuffer: return ShaderResourceType::UniformBuffer;
            case slang::BindingType::RawBuffer:
            case slang::BindingType::TypedBuffer: return ShaderResourceType::ReadonlyBuffer;
            case slang::BindingType::MutableTexture: return ShaderResourceType::WritableImage;
            case slang::BindingType::MutableRawBuffer:
            case slang::BindingType::MutableTypedBuffer: return ShaderResourceType::WritableBuffer;
            default: throw std::runtime_error("unsupported shader resource, slang::BindingType: " + std::to_string(static_cast<uint32_t>(type)));
        };
    }

    static constexpr ShaderStageBits SlangToGE(const SlangStage stage) {
        switch (stage) {
            case SLANG_STAGE_VERTEX: return ShaderStageBits::Vertex;
            case SLANG_STAGE_FRAGMENT: return ShaderStageBits::Fragment;
            case SLANG_STAGE_COMPUTE: return ShaderStageBits::Compute;
            default: throw std::runtime_error("unsupported shader stage, SlangStage: " + std::to_string(static_cast<uint32_t>(stage)));
        };
    }

    static GEString ReadShaderFile(const std::filesystem::path& path) {
        std::ifstream file(path);

        return { std::istreambuf_iterator<char>(file),
                std::istreambuf_iterator<char>() };
    }

    static GEUnorderedMap<GEString, ShaderResource> GetResources(slang::ProgramLayout* layout) {
        GEUnorderedMap<GEString, ShaderResource> resources;
        resources.reserve(layout->getParameterCount());
        for (const auto i : Counter(layout->getParameterCount())) {
            auto* var_layout = layout->getParameterByIndex(i);

            if (layout->getParameterByIndex(i)->getBindingSpace() != 0) continue;

            resources.emplace(
                var_layout->getName(),
                ShaderResource{
                    var_layout->getName(),
                    static_cast<uint16_t>(var_layout->getTypeLayout()->getBindingRangeBindingCount(0)),
                    static_cast<uint16_t>(layout->getParameterByIndex(i)->getBindingIndex()),
                    SlangToGE(var_layout->getTypeLayout()->getBindingRangeType(0)),
                }
            );
        }
        return resources;
    }

    static ShaderEntryPoint GetEntryPoint(slang::EntryPointReflection* entryPointRefl) {
        return {
            entryPointRefl->getName(),
            SlangToGE(entryPointRefl->getStage()),
        };
    }

    static GEUnorderedMap<GEString, ShaderEntryPoint> GetEntryPoints(slang::ProgramLayout* programLayout) {
        GEUnorderedMap<GEString, ShaderEntryPoint> out;
        for (const auto i : Counter(programLayout->getEntryPointCount())) {
            auto entryPoint = GetEntryPoint(programLayout->getEntryPointByIndex(i));
            out.try_emplace(entryPoint.name, entryPoint);
        }
        return out;
    }

    static ShaderReflection GetReflection(slang::ProgramLayout* programLayout) {
        return {
            GetEntryPoints(programLayout),
            GetResources(programLayout),
        };
    }

    static GEVector<char> GetSpirvCode(Slang::ComPtr<slang::IComponentType> linkedProgram) {
        Slang::ComPtr<slang::IBlob> blob;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            linkedProgram->getTargetCode(
                SpirvTargetIndex,
                blob.writeRef(),
                diagnosticsBlob.writeRef());

            if (!blob) {
                GE_GRAPHICS_WARN("Shader module loading error: {}",
                    std::string_view(static_cast<const char*>(diagnosticsBlob->getBufferPointer()), diagnosticsBlob->getBufferSize()));
            }
        }

        return { (char*)blob->getBufferPointer(), (char*)blob->getBufferPointer() + blob->getBufferSize() };
    }

    // TODO: complate this func
    static GEVector<char> GetDxilCode(Slang::ComPtr<slang::IComponentType> linkedProgram, uint32_t entryPointCount) {
        GE_ASSERT(false, "Not completed func!!!");
        GEVector<char> out{};
        {
            Slang::ComPtr<slang::IBlob> blob;
            for (const auto entryPointIndex : Counter(entryPointCount))
            {
                Slang::ComPtr<slang::IBlob> diagnosticsBlob;
                linkedProgram->getEntryPointCode(
                    entryPointIndex,
                    DxilTargetIndex,
                    blob.writeRef(),
                    nullptr
                );

                out.push_back(blob->getBufferSize());
                out.insert(out.end(), blob->getBufferSize(), blob->getBufferSize() + blob->getBufferSize());
            }
        }
        return out;
    }

    bool CompileShader(const std::filesystem::path& shaderDir, std::string_view shaderName, ShaderData& shaderData) {
        Slang::ComPtr<slang::IGlobalSession> globalSession;
        Slang::ComPtr<slang::ISession> session;

        slang::createGlobalSession(globalSession.writeRef());
        slang::SessionDesc sessionDesc = {};

        std::array<slang::TargetDesc, 2> targetDesc;
        targetDesc[SpirvTargetIndex].format = SLANG_SPIRV;
        targetDesc[SpirvTargetIndex].profile = globalSession->findProfile("spirv_1_3");

        targetDesc[DxilTargetIndex].format = SLANG_DXIL;
        targetDesc[DxilTargetIndex].profile = globalSession->findProfile("sm_6_6");

        std::array<slang::CompilerOptionEntry, 2> options;
        options[0].name = slang::CompilerOptionName::Optimization;
        options[0].value = { slang::CompilerOptionValueKind::Int, 3 };
        options[1].name = slang::CompilerOptionName::EmitSpirvDirectly;
        options[1].value = { slang::CompilerOptionValueKind::Int, 1 };

        sessionDesc.targets = targetDesc.data();
        sessionDesc.targetCount = targetDesc.size();

        sessionDesc.compilerOptionEntries = options.data();
        sessionDesc.compilerOptionEntryCount = options.size();

        globalSession->createSession(sessionDesc, session.writeRef());

        const auto shaderPath = shaderDir / (GEString(shaderName) + ".slang");
        if (!std::filesystem::exists(shaderPath)) {
            GE_GRAPHICS_WARN("Shader not found: {}", shaderName);
            return false;
        }

        Slang::ComPtr<slang::IModule> slang_module;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;

            slang_module = session->loadModuleFromSourceString(
                shaderName.data(),
                shaderPath.string().c_str(),
                ReadShaderFile(shaderPath).c_str(),
                diagnosticsBlob.writeRef()
            );

            if (!slang_module) {
                GE_GRAPHICS_WARN("Shader module loading error: {}",
                    std::string_view(static_cast<const char*>(diagnosticsBlob->getBufferPointer()), diagnosticsBlob->getBufferSize()));
                return false;
            }
        }

        GEVector<slang::IComponentType*> componentTypes = {
            slang_module,
        };

        GEVector<Slang::ComPtr<slang::IEntryPoint>> entryPoints(slang_module->getDefinedEntryPointCount());
        for (const auto i : Counter(slang_module->getDefinedEntryPointCount())) {
            slang_module->getDefinedEntryPoint(i, entryPoints[i].writeRef());
            componentTypes.emplace_back(entryPoints[i]);
        }

        Slang::ComPtr<slang::IComponentType> composedProgram;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            session->createCompositeComponentType(
                componentTypes.data(),
                componentTypes.size(),
                composedProgram.writeRef(),
                diagnosticsBlob.writeRef());
            if (diagnosticsBlob) {
                GE_GRAPHICS_WARN("Shader Compose error: {}",
                    std::string_view(static_cast<const char*>(diagnosticsBlob->getBufferPointer()), diagnosticsBlob->getBufferSize()));
                return false;
            }
        }

        Slang::ComPtr<slang::IComponentType> linkedProgram;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            SlangResult result = composedProgram->link(
                linkedProgram.writeRef(),
                diagnosticsBlob.writeRef());
            if (diagnosticsBlob) {
                GE_GRAPHICS_WARN("Shader linking error: {}",
                    std::string_view((char*)diagnosticsBlob->getBufferPointer(),
                        diagnosticsBlob->getBufferSize()));
                return false;
            }
        }

        try {
            //shaderData.reflection = GetReflection(linkedProgram->getLayout());
            shaderData.reflection = {};// GetReflection(linkedProgram->getLayout());
            shaderData.spirvByteCode = GetSpirvCode(linkedProgram);
            //shaderData.dxilByteCodes = GetSpirvCode(linkedProgram);
            return true;
        }
        catch (const std::exception& e) {
            GE_GRAPHICS_WARN("shader compile error, shader: {}, error: {}", shaderName, e.what());
            return false;
        }
    }
}
