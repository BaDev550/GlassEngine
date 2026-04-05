#pragma once
#include "GlassEngine/Renderer/Shader.h"
#include <unordered_map>

namespace ge::renderer {
	class ShaderLibrary {
	public:
		mem::Ref<Shader>& AddShader(const GEString& name, const GEString& vertexPath, const GEString& fragmentPath) {
			_loadedShaders[name] = Shader::Create(vertexPath, fragmentPath);
			_loadedShaders[name]->Compile(); // TODO (0x): compile all the shaders on difirent thread
			return _loadedShaders[name];
		}
		mem::Ref<Shader>& GetShader(const GEString& name) { return _loadedShaders[name]; }
	private:
		std::unordered_map<GEString, mem::Ref<Shader>> _loadedShaders;
	};
}