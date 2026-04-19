#pragma once
#include "GlassEngine/Renderer/Shader.h"
#include "ShaderCompiler.h"
#include <unordered_map>

namespace ge::renderer {
	class ShaderLibrary {
	public:
		ge::mem::Ref<Shader>& AddShader(const GEString& shaderName) {
			ShaderData _shaderData;
			CompileShader("Shaders/", shaderName, _shaderData);
			_loadedShaders[shaderName] = Shader::Create(shaderName, std::move(_shaderData));
			return _loadedShaders[shaderName];
		}

		ge::mem::Ref<Shader>& GetShader(const GEString& name) { return _loadedShaders[name]; }
		void ReloadShader(const GEString& name) {
			if (_loadedShaders.find(name) != _loadedShaders.end()) {
				
			}
		}
		void ReloadAll() {
			for (auto& [name, shader] : _loadedShaders) {
				
			}
		}
	private:
		std::unordered_map<GEString, ge::mem::Ref<Shader>> _loadedShaders;
	};
}