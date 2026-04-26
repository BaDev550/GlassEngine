#pragma once
#include "GlassEngine/Serialization/FileSerializer.h"
#include "GlassEngine/Core/Core.h"
#include "Scene.h"

namespace ge {
	class SceneSerializer {
	public:
		SceneSerializer(const mem::Ref<Scene>& scene);
		bool Serialize(const GEString& path);
		bool Deserialize(const GEString& path);
	private:
		mem::Ref<Scene> _scene;
	};
}