#include "gepch.h"
#include "Pipeline.h"

#include "Platform/Vulkan/Vulkan_Pipeline.h"

namespace ge::renderer {
	ge::mem::Ref<Pipeline> Pipeline::Create(const PipelineSpecification& specs)
	{
		switch (RenderAPI::GetAPI())
		{
		case GraphicsAPI::Vulkan: return ge::mem::Ref<Vulkan_Pipeline>::Create(specs);
		case GraphicsAPI::DirectX11: return nullptr;
		case GraphicsAPI::OpenGL: return nullptr;
		default:
			GE_CORE_ERROR("Failed to select api");
			break;
		}
		return nullptr;
	}
}