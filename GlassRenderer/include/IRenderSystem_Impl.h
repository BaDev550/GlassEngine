#pragma once

#include <Graphics/Interfaces/IRenderSystem.h>

namespace ge::graphics {
	class IRenderSystem_Impl : public IRenderSystem {
	public:
		virtual void Initialize() override;
		virtual void Destroy() override;

		virtual void DrawVertex(uint32_t vertexCount, const mem::Ref<IBuffer>& vertexBuffer) {};
		virtual void DrawIndex(uint32_t indexCount, const mem::Ref<IBuffer>& vertexBuffer, const mem::Ref<IBuffer>& indexBuffer) {};
		virtual void PushConstant(const void* ptr, uint16_t size, uint16_t offset) {};

		virtual void BeginCopyPass() override {}
		virtual void EndCopyPass() override {}

		virtual void BeginDebugLabel(std::string_view label) override {}
		virtual void EndDebugLabel() override {};

		virtual const char* GetDebugName() override { return "GlassRenderer_Interface_Render_system_Impl"; };
	};
	extern "C" IRenderSystem* CreateRenderSystem() { return new IRenderSystem_Impl(); }
}