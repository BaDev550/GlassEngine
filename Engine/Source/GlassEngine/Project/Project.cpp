#include "gepch.h"
#include "Project.h"
#include "GlassEngine/Core/Application.h"
#include "GlassEngine/Asset/AssetManager.h"
#include "GlassEngine/Project/ProjectSerializer.h"

namespace ge {
	mem::Ref<Project> Project::_activeProject = nullptr;
	mem::Ref<Project> Project::New(const GEString& name, const std::filesystem::path& dir)
	{
		_activeProject = mem::Ref<Project>::Create();
		ProjectSerializer serializer(_activeProject);
		serializer.CreateProjectDirectories(name, dir);
		_activeProject->_projectDirectory = dir;
		_activeProject->_projectPath = (dir / name.ToPath());
		return _activeProject;
	}

	mem::Ref<Project> Project::Load(const std::filesystem::path& path) {
		mem::Ref<Project> project = mem::Ref<Project>::Create();
		ProjectSerializer serializer(project);

		if (serializer.Deserialize(path)) {
			project->_projectDirectory = path.parent_path();
			project->_projectPath = path;
			_activeProject = std::move(project);

			AssetManager::Init(_activeProject.Get());

			return _activeProject;
		}
	}

	bool Project::SaveActive()
	{
		ProjectSerializer serializer(_activeProject);
		return serializer.Serialize(_activeProject->_projectPath);
	}
}