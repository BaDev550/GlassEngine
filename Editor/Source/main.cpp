#include <iostream>
#include <GlassEngine/Core/EntryPoint.h>
#include <GlassEngine/Core/Application.h>
#include <GlassEngine/Layers/Layer.h>
#include <GlassEngine/Memory/Ref.h>
#include <GlassEngine/Core/Core.h>
#include <GlassEngine/Renderer/Renderer.h>
#include <GlassEngine/Renderer/Model.h>
#include <GlassEngine/Scene/Scene.h>
#include <GlassEngine/Editor/EditorConsole.h>
#include <GlassEngine/Editor/EditorECSDebugPanel.h>

#include <imgui.h>

class EditorLayer : public ge::Layer {
public:
	EditorLayer() : ge::Layer("LAYER_Editor") {}
	virtual void OnAttach() override {
		_scene = ge::mem::Ref<ge::Scene>::Create("Test Scene");

		GetPanelManager().RegisterPanel<ge::editor::Console>("E_c");
		GetPanelManager().RegisterPanel<ge::editor::EditorECSDebugPanel>("E_ecs", _scene.Get());

		GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.showPanel E_c");
		GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.showPanel E_ecs");

#if 1
		auto mario = ge::Application::Get()->GetAssetManager()->GetOrImportAsset("Resources/mario_2/mario_2.obj").Cast<ge::renderer::StaticMesh>();
		for (int i = 0; i < mario->GetLODs().size(); i++) {
			auto& lod = mario->GetLODs()[i];
			GE_APPLICATION_INFO("LOD: {}", i);
			GE_APPLICATION_INFO(" -vertexC: {}", lod.vertices.size());
			GE_APPLICATION_INFO(" -indicesC: {}", lod.indices.size());
			GE_APPLICATION_INFO(" -submeshC: {}", lod.submesh.size());
		}
#endif
	}

	virtual void OnDetach() override {}
	virtual void OnUpdate(float deltaTime) override {
		ge::renderer::Renderer3D::BeginDefaultPass(); // TODO (badev): Move this into scene renderer

		_scene->OnEditorUpdate(deltaTime);

		ge::renderer::Renderer3D::EndDefaultPass();
	}

	virtual void OnImGuiRender() override {}
private:
	ge::mem::Ref<ge::Scene> _scene;
};

class EditorApp : public ge::Application {
public:
	EditorApp(const ge::ApplicationSpecification& createInfo) : ge::Application(createInfo) {
		GE_APPLICATION_INFO("EditorApp created!");
		PushLayer(new EditorLayer());
	}
};

namespace ge {
	Application* CreateApplication(const ApplicationSpecification& createInfo) {
		return new EditorApp(createInfo);
	}
}