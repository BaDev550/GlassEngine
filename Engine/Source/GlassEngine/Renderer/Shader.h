#pragma once
#include "GlassEngine/Renderer/RenderObject.h"
#include "GlassEngine/Renderer/Types.h"

#include <span>

namespace ge::renderer {
	class Shader : public RenderObject, public Asset {
	public:
		Shader(std::string_view shaderName, ShaderData&& shaderData)
			: _shaderName(shaderName), _shaderData(std::move(shaderData)) {}
		~Shader() {}
		virtual AssetType GetAssetType() const override final { return AssetType::SourceShader; }

		[[nodiscard]] std::string_view GetShaderName() const noexcept { return _shaderName; }
		[[nodiscard]] const ShaderReflection& GetReflection() const noexcept { return _shaderData.reflection; }
		[[nodiscard]] auto GetSpirvByteCode() const noexcept { return std::span(_shaderData.spirvByteCode); }
		[[nodiscard]] const auto& GetDxilByteCodes() const noexcept { return _shaderData.dxilByteCodes; }

		[[nodiscard]] auto GetDxilByteCode(std::string_view entryPoint) const noexcept {
			const auto it = _shaderData.dxilByteCodes.find(GEString(entryPoint));
			return it == _shaderData.dxilByteCodes.end() ? std::span<char>{} : std::span(it->second);
		}

		static ge::mem::Ref<Shader> Create(std::string_view shaderName, ShaderData&& shaderData);
	protected:
		const GEString _shaderName;
		const ShaderData _shaderData;
	};
}