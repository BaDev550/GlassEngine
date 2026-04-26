#include "gepch.h"
#include "Project.h"
#include "GlassEngine/Core/Application.h"
#include "GlassEngine/Asset/AssetManager.h"

namespace ge {
	mem::Ref<Project> Project::_activeProject = nullptr;
	mem::Ref<Project> Project::New()
	{
		return mem::Ref<Project>();
	}
	mem::Ref<Project> Project::Load(const std::filesystem::path& path)
	{
		return mem::Ref<Project>();
	}
	bool Project::SaveActive()
	{
		return false;
	}
}