#pragma once

#include "GlassEngine/Core/Memory.h"
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Utilities/Flags.h"
#include "Shader.h"
#include "Types.h"
#include <span>
#include <filesystem>

namespace ge::renderer {
	class ShaderCompiler {
	public:
		ShaderCompiler(std::filesystem::path&& shaderDir) : _shaderDir(std::move(shaderDir)) {}
		ShaderCompiler(const std::filesystem::path& shaderDir) : _shaderDir(shaderDir) {}

		void CompileShaders(std::span<const std::string> shaderNames);
		bool CompileShader(std::string_view shaderName); // compile the shader, add the _shaderDatas and write the shaderData to disk, returns shader is exists
		// TODO (0x): write this func
		bool LoadShaderDataFromDisk(std::string_view shaderName); // loads shader data and adds the _shaderDatas, returns shader is exists
		[[nodiscard]] const ShaderData* GetShaderData(std::string_view shaderName) noexcept;
	private:
		std::filesystem::path _shaderDir;
		std::unordered_map<std::string, ShaderData> _shaderDatas;
	};

	inline const ShaderData* ShaderCompiler::GetShaderData(std::string_view shaderName) noexcept {
		auto shader = std::string(shaderName);
		auto [shaderDataIt, is_emplaced] = _shaderDatas.try_emplace(shader, ShaderData{});
		// shaderData exists
		if (!is_emplaced) return &(*shaderDataIt).second;
		// shaderData exists in disk
		if (LoadShaderDataFromDisk(shaderName)) return &(*shaderDataIt).second;
		// shaderData exists in slang shader
		else if (CompileShader(shaderName)) &(*shaderDataIt).second;
		return nullptr;
	}

	inline bool ShaderCompiler::CompileShader(std::string_view shaderName) {
		// TODO (0x): check shader exists
		auto name = std::string(shaderName);
		CompileShaders(std::span(&name, 1));
		return true;
	}
}