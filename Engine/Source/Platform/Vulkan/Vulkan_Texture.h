#pragma once
#include "GlassEngine/Renderer/Texture.h"

namespace ge::renderer {
	class Vulkan_Texture2D : public Texture2D {
	public:
		Vulkan_Texture2D(const TextureSpecification& spec);
		Vulkan_Texture2D(const TextureSpecification& spec, const void* data);
	};
}