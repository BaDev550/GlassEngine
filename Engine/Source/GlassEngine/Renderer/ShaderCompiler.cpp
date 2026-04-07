#include "ShaderCompiler.h"
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <slang/slang-com-helper.h>
#include <GlassEngine/Utilities/Counter.h>

namespace ge::renderer {
    static constexpr uint32_t SPIRV_TARGET_INDEX = 0;
    static constexpr uint32_t DXIL_TARGET_INDEX = 1;

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

    static std::vector<char> ReadShaderFile(const std::filesystem::path& path) {
        std::ifstream file(path);

        return { std::istreambuf_iterator<char>(file),
                std::istreambuf_iterator<char>() };
    }

    static std::unordered_map<GEString, ShaderResource> GetResources(slang::ProgramLayout* layout) {
        std::unordered_map<GEString, ShaderResource> resources;
        resources.reserve(layout->getParameterCount());
        for (const auto i : Counter(layout->getParameterCount())) {
            auto* var_layout = layout->getParameterByIndex(i);

            const auto type = SlangToGE(var_layout->getTypeLayout()->getBindingRangeType(0));

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

    static std::unordered_map<GEString, ShaderEntryPoint> GetEntryPoints(slang::ProgramLayout* programLayout) {
        std::unordered_map<GEString, ShaderEntryPoint> out;
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

    static std::vector<char> GetSpirvCode(Slang::ComPtr<slang::IComponentType> linkedProgram) {
        Slang::ComPtr<slang::IBlob> blob;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            linkedProgram->getTargetCode(
                SPIRV_TARGET_INDEX,
                blob.writeRef());
        }

        return { (char*)blob->getBufferPointer(), (char*)(blob->getBufferPointer()) + blob->getBufferSize() };
    }

    // TODO: complate this func
    static std::vector<char> GetDxilCode(Slang::ComPtr<slang::IComponentType> linkedProgram, uint32_t entryPointCount) {
        GE_ASSERT(false, "Not completed func!!!")
        std::vector<char> out{};
        {
            Slang::ComPtr<slang::IBlob> blob;
            for (const auto entryPointIndex : Counter(entryPointCount))
            {
                Slang::ComPtr<slang::IBlob> diagnosticsBlob;
                linkedProgram->getEntryPointCode(
                    entryPointIndex,
                    DXIL_TARGET_INDEX,
                    blob.writeRef(),
                    nullptr
                );

                out.push_back(blob->getBufferSize());
                out.insert(out.end(), blob->getBufferSize(), blob->getBufferSize() + blob->getBufferSize());
            }
        }
        return out;
    }

	void ShaderCompiler::CompileShaders(std::span<const std::string> shaderNames) {
		Slang::ComPtr<slang::IGlobalSession> globalSession;
		Slang::ComPtr<slang::ISession> session;

		slang::createGlobalSession(globalSession.writeRef());
        slang::SessionDesc sessionDesc = {};

        std::array<slang::TargetDesc, 2> targetDesc;
        targetDesc[SPIRV_TARGET_INDEX].format = SLANG_SPIRV;
        targetDesc[SPIRV_TARGET_INDEX].profile = globalSession->findProfile("spirv_1_3");

        targetDesc[DXIL_TARGET_INDEX].format = SLANG_DXIL;
        targetDesc[DXIL_TARGET_INDEX].profile = globalSession->findProfile("sm_6_6");

        std::array<slang::CompilerOptionEntry, 2> options;
        options[0].name = slang::CompilerOptionName::Optimization;
        options[0].value = { slang::CompilerOptionValueKind::Int, 3 };
        options[0].name = slang::CompilerOptionName::EmitSpirvDirectly;
        options[0].value = { slang::CompilerOptionValueKind::Int, 1 };

        sessionDesc.targets = targetDesc.data();
        sessionDesc.targetCount = targetDesc.size();

        sessionDesc.compilerOptionEntries = options.data();
        sessionDesc.compilerOptionEntryCount = options.size();

        globalSession->createSession(sessionDesc, session.writeRef());

        // TODO (0x): use multithread
        for (const auto& shaderName : shaderNames) {
            const auto shaderPath = _shaderDir / (shaderName + ".slang");
            if (!std::filesystem::exists(shaderPath)) {
                GE_GRAPHICS_WARN("Shader not found: ", shaderName);
                continue;
            }

            Slang::ComPtr<slang::IModule> slang_module;
            {
                Slang::ComPtr<slang::IBlob> diagnosticsBlob;

                slang_module = session->loadModuleFromSourceString(
                    shaderName.c_str(),
                    shaderPath.string().c_str(),
                    ReadShaderFile(shaderPath).data(),
                    diagnosticsBlob.writeRef()
                );
                
                if (!slang_module) {
                    GE_GRAPHICS_WARN("Shader module loading error: {}",
                        std::string_view(static_cast<const char*>(diagnosticsBlob->getBufferPointer()), diagnosticsBlob->getBufferSize()));
                    continue;
                }
            }

            std::vector<slang::IComponentType*> componentTypes = {
                slang_module,
            };

            std::vector<Slang::ComPtr<slang::IEntryPoint>> entryPoints(slang_module->getDefinedEntryPointCount());
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
                    continue;
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
                    continue;
                }
            }

            try {
                ShaderData shaderData;
                shaderData.reflection = GetReflection(linkedProgram->getLayout());
                switch (RenderAPI::GetAPI())
                {
                case GraphicsAPI::Vulkan: shaderData.byteCode = GetSpirvCode(linkedProgram);
                case GraphicsAPI::DirectX11: GE_CORE_ERROR("Shader compiler don't support D3D11 backend");
                case GraphicsAPI::DirectX12: GE_CORE_ERROR("Shader compiler don't support D3D12 backend");
                case GraphicsAPI::OpenGL: GE_CORE_ERROR("Shader compiler don't support OpenGL backend");
                default:
                    GE_CORE_ERROR("Failed to select api");
                    break;
                }
                _shaderDatas[shaderName] = shaderData;
            }
            catch (const std::exception& e) {
                GE_GRAPHICS_WARN("shader compile error, shader: {}, error: {}", shaderName, e.what());
            }
        }
	}
}