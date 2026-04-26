#include <iostream>
#include <GlassEngine/Core/EntryPoint.h>
#include <GlassEngine/Core/Application.h>
#include <GlassEngine/Layers/Layer.h>
#include <GlassEngine/Memory/Ref.h>
#include <GlassEngine/Core/Core.h>
#include <GlassEngine/Renderer/Renderer.h>
#include <GlassEngine/Renderer/Model.h>
#include <GlassEngine/Renderer/FreeCamera.h>
#include <GlassEngine/Scene/Scene.h>
#include <GlassEngine/Editor/EditorConsole.h>
#include <GlassEngine/Editor/EditorECSDebugPanel.h>

#include <imgui.h>

class EditorLayer : public ge::Layer {
public:
	EditorLayer() : ge::Layer("LAYER_Editor") {}
	virtual void OnAttach() override {
		_scene = ge::mem::Ref<ge::Scene>::Create("Test Scene");
		_camera = ge::mem::Ref<ge::renderer::FreeCamera>::Create();

		GetPanelManager().RegisterPanel<ge::editor::Console>("E_c");
		GetPanelManager().RegisterPanel<ge::editor::EditorECSDebugPanel>("E_ecs", _scene.Get());

		GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.showPanel E_c");
		GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.showPanel E_ecs");
#if 1
		auto mario = ge::AssetManager::GetOrImportAsset<ge::renderer::StaticMesh>("Resources/DamagedHelmet/DamagedHelmet.gltf");
		for (int i = 0; i < mario->GetLODs().size(); i++) {
			auto& lod = mario->GetLODs()[i];
			GE_APPLICATION_INFO("LOD: {}", i);
			GE_APPLICATION_INFO(" -vertexC: {}", lod.vertices.size());
			GE_APPLICATION_INFO(" -indicesC: {}", lod.indices.size());
			GE_APPLICATION_INFO(" -submeshC: {}", lod.submesh.size());
		}

		entity = _scene->CreateEntity("Mario");
		auto& smc = entity->AddComponent<ge::StaticMeshComponent>();
		smc.meshHandle = mario->_assetHandle;
#endif
	}

	virtual void OnDetach() override {}
	virtual void OnUpdate(float deltaTime) override {
		_camera->Update(deltaTime);

		_scene->OnEditorUpdate(deltaTime, _camera);

		if (ge::Input::IsKeyJustPressed(ge::key::Tab)) {
			_cursor = !_cursor;
			_camera->SetProccessingMouse(!_cursor);
			_camera->SetFirstMouse();
			ge::Application::Get()->GetWindow().SetCursor(_cursor);
		}
	}

	virtual void OnImGuiRender() override {}
private:
	ge::mem::Ref<ge::Scene> _scene;
	ge::mem::Ref<ge::renderer::FreeCamera> _camera;

	bool _cursor = false;
	ge::Entity* entity;
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