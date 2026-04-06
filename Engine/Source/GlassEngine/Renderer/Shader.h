#pragma once
#include "GlassEngine/Renderer/RenderObject.h"
#include "GlassEngine/Renderer/Types.h"

namespace ge::renderer {
	enum class ShaderStat : uint8_t {
		None = 0,
		LoadedCompiled
	};

	struct DescriptorInfo {
		std::string name;
		ShaderDataType type;
		uint32_t count = 1;
	};

	class Shader : public RenderObject {
	public:
		virtual ~Shader() = default;
		virtual void Compile() = 0; 

		ShaderStat GetShaderStage() const { return _shaderStage; }
		bool IsReady() const { return (_shaderStage == ShaderStat::LoadedCompiled); }
		static ge::mem::Ref<Shader> Create(const GEString& vertexPath, const GEString& fragmentPath);
	protected:
		ShaderStat _shaderStage = ShaderStat::None;
	};
}