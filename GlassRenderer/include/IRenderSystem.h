#pragma once

#include <Renderer/Interfaces/IRenderSystem.h>

namespace ge::renderer {
	class IRenderSystem_Impl : public IRenderSystem {
	public:
		virtual void DrawVertex(uint32_t vertexCount, const mem::Ref<IBuffer>& vertexBuffer) override {};
		virtual void DrawIndex(uint32_t indexCount, const mem::Ref<IBuffer>& vertexBuffer, const mem::Ref<IBuffer>& indexBuffer) override {};

		virtual void PushConstant(const void* ptr, uint16_t size, uint16_t offset) override {};

		virtual void BeginCopyPass() override {}
		virtual void EndCopyPass() override {}

		virtual void BeginDebugLabel(std::string_view label) override {}
		virtual void EndDebugLabel() override {};

		virtual const char* GetDebugName() override { return "GlassRenderer_Interface_Render_system_Impl"; };
	};
	extern "C" GE_API IRenderSystem* CreateRenderSystem() { return new IRenderSystem_Impl(); }
}