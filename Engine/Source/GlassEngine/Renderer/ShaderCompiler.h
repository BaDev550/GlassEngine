#pragma once

#include "GlassEngine/Core/Memory.h"
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Utilities/Flags.h"
#include "Shader.h"
#include "Types.h"
#include <span>
#include <filesystem>
#include <string_view>
#include <array>

namespace ge::renderer {
	bool CompileShader(const std::filesystem::path& shaderDir, std::string_view shaderName, ShaderData& shaderData);
}