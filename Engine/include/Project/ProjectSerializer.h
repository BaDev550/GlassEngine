#pragma once
#include "Project.h"

namespace ge {
	class ProjectSerializer
	{
	public:
		ProjectSerializer(mem::Ref<Project> project);

		bool Serialize(const filesystem::Path& path);
		bool Deserialize(const filesystem::Path& path);

		static bool CreateProjectDirectories(const GEString& name, const filesystem::Path& dir);
	private:
		static bool SerializeWithConfig(const filesystem::Path& path, const ProjectConfig& config);
		static bool CreateCMakeFile(const filesystem::Path& cmakePath, const ProjectConfig& config);
		mem::Ref<Project> _Project;
	};
}