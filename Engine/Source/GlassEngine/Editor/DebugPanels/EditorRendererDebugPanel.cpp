#include "gepch.h"
#include "GlassEngine/Renderer/RenderAPI.h"
#include "GlassEngine/Renderer/Renderer.h"
#include "EditorRendererDebugPanel.h"

#include <imgui.h>

namespace ge::editor {
	void EditorRendererDebugPanel::Draw()
	{
		auto renderStats = renderer::RenderAPI::GetRenderStats();
		auto renderConfig = renderer::Renderer3D::GetRenderConfig();
		
		ImGui::Begin("Renderer Debug Panel"); // TODO(badev): move this into main editor panel and take a name
		ImGui::Text("Draw calls: %d", renderStats.drawCalls);
		ImGui::Separator();
		ImGui::Text("Render Config");
		ImGui::Text("FPS: %f", (1.0f / Engine::Get().GetDeltaTime()));
		ImGui::Text("Vsync: %d", renderConfig.vsync);
		ImGui::Text("Wireframe: %d", renderConfig.wireframe);
		ImGui::End();
	}
}