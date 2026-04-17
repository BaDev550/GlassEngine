#pragma once
#include "GlassEngine/Renderer/Shader.h"
#include <unordered_map>

namespace ge::renderer {
	class ShaderLibrary {
	public:
		ge::mem::Ref<Shader>& AddShader(const GEString& name, const GEString& vertexPath, const GEString& fragmentPath) {
			_loadedShaders[name] = Shader::Create(vertexPath, fragmentPath);
			_loadedShaders[name]->Compile(); // TODO (0x): compile all the shaders on difirent thread
			return _loadedShaders[name];
		}
		ge::mem::Ref<Shader>& GetShader(const GEString& name) { return _loadedShaders[name]; }
	private:
		std::unordered_map<GEString, ge::mem::Ref<Shader>> _loadedShaders;
	};
}