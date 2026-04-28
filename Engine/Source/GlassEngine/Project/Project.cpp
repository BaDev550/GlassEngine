#include "gepch.h"
#include "Project.h"
#include "GlassEngine/Core/Application.h"
#include "GlassEngine/Asset/AssetManager.h"
#include "GlassEngine/Project/ProjectSerializer.h"

namespace ge {
	mem::Ref<Project> Project::_activeProject = nullptr;
	mem::Ref<Project> Project::New()
	{
		return mem::Ref<Project>();
	}
	mem::Ref<Project> Project::Load(const std::filesystem::path& path) {
		mem::Ref<Project> project = nullptr;
		ProjectSerializer serializer(project);

		if (serializer.Deserialize(path)) {
			project->_projectDirectory = path.parent_path();
			project->_projectPath = path;
			_activeProject = std::move(project);
			return nullptr;
		}
	}
	bool Project::SaveActive()
	{
		return false;
	}
}