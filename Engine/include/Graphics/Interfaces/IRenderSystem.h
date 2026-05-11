#pragma once
#include "IEngineSystem.h"

#include "IBuffer.h"

namespace ge::graphics {
#define GRAPHICS_API_ENUM ::ge::Engine::Get().GetRenderSystem()->GetAPI()

	enum class GraphicsAPI : uint8_t {
		Vulkan = 0,
		DirectX12
	};

	struct RenderStats {
		uint32_t drawCall;
	};

	class GE_API IRenderSystem {
	public:
		IRenderSystem() { _logger_renderSystem = mem::Ref<Logger>::Create("RENDER_SYS"); }
		virtual ~IRenderSystem() = default;
		virtual void Initialize() = 0;
		virtual void Destroy() = 0;

		virtual void BeginCopyPass() = 0;
		virtual void EndCopyPass() = 0;

		Logger& GetLogger() { return *_logger_renderSystem; }
		RenderStats GetRenderStats() { return _renderStats; }
		GraphicsAPI GetAPI() { return _api; }
		void SetAPI(GraphicsAPI api) { _api = api; }

		virtual void BeginDebugLabel(std::string_view label) = 0;
		virtual void EndDebugLabel() = 0;

		virtual const char* GetDebugName() { return "Interface_Render_system"; };
	protected:
		GraphicsAPI _api;
		RenderStats _renderStats;
		mem::Ref<Logger> _logger_renderSystem;
	};
	extern "C" { typedef IRenderSystem* (*CreateRenderSystemFunc)(); }
}