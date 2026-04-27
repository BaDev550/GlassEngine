#pragma once
#include "Project.h"

namespace ge {
	class ProjectSerializer
	{
	public:
		ProjectSerializer(mem::Ref<Project> project);

		bool Serialize(const std::filesystem::path& path);
		bool Deserialize(const std::filesystem::path& path);

		static bool CreateProjectDirectories(const std::string& name, const std::filesystem::path& dir);
	private:
		static bool SerializeWithConfig(const std::filesystem::path& path, const ProjectConfig& config);
		static bool CreateCMakeFile(const std::filesystem::path& cmakePath, const ProjectConfig& config);
		mem::Ref<Project> _Project;
	};
}