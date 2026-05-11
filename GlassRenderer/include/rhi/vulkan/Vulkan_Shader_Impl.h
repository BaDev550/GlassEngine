#pragma once
#include "GlassEngine/Renderer/Shader.h"
#include "Vulkan_RenderContext.h"
#include "Vulkan_Types.h"
#include <map>

namespace ge::renderer {
	class Vulkan_Shader : public Shader {
	public:
		Vulkan_Shader(std::string_view shaderName, ShaderData&& shaderData)
			: Shader(shaderName, std::move(shaderData)) {}
		~Vulkan_Shader() = default;
		virtual void SetDebugName(GEString name) const noexcept override {}
	};
}