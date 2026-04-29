#include <iostream>
#include <GlassEngine/Core/EntryPoint.h>
#include <GlassEngine/Core/Application.h>
#include <GlassEngine/Layers/Layer.h>
#include <GlassEngine/Memory/Ref.h>
#include <GlassEngine/Core/Core.h>
#include <GlassEngine/Core/String.h>
#include <GlassEngine/Renderer/Renderer.h>
#include <GlassEngine/Renderer/Model.h>
#include <GlassEngine/Renderer/FreeCamera.h>
#include <GlassEngine/Scene/Scene.h>
#include <GlassEngine/Scene/SceneSerializer.h>
#include <GlassEngine/Editor/EditorConsole.h>
#include <GlassEngine/Editor/Debug/EditorECSDebugPanel.h>
#include <GlassEngine/Editor/Debug/EditorRendererDebugPanel.h>
#include <GlassEngine/Editor/Debug/EditorMemoryDebugPanel.h>
#include <GlassEngine/Editor/ModelImportPanel.h>
#include <GlassEngine/Project/ProjectSerializer.h>

#include <imgui.h>

class EditorLayer : public ge::Layer {
public:
	EditorLayer() : ge::Layer("LAYER_Editor") {}
	virtual void OnAttach() override {
		_camera = ge::mem::Ref<ge::renderer::FreeCamera>::Create();
		OpenProject("projects/sandbox/sandbox.geproj");

		GetPanelManager().RegisterPanel<ge::editor::Console>("E_c");
		GetPanelManager().RegisterPanel<ge::editor::EditorECSDebugPanel>("E_ecs", _scene.Get());
		GetPanelManager().RegisterPanel<ge::editor::EditorRendererDebugPanel>("E_r");
		GetPanelManager().RegisterPanel<ge::editor::EditorMemoryDebugPanel>("E_m");
		GetPanelManager().RegisterPanel<ge::editor::ModelImportPanel>("E_mI");

		GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.showPanel E_c");
		GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.showPanel E_ecs");
		GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.showPanel E_r");

		GE_ADD_CONSOLE_COMMAND("editor", "save_scene", [this](const GEVector<GEString>& args) {
			ge::SceneSerializer serializer(_scene);
			serializer.Serialize(args[0]);
			}, "editor.save_scene <path>");

		GE_ADD_CONSOLE_COMMAND("editor", "load_scene", [this](const GEVector<GEString>& args) { 
			ge::SceneSerializer serializer(_scene);
			serializer.Deserialize(args[0]);
			}, "editor.load_scene <path>");
		GE_ADD_CONSOLE_COMMAND("editor", "create_project", [this](const GEVector<GEString>& args) {
			NewProject(args[0], args[1].ToPath());
			}, "editor.create_project <name> <directory>");
		GE_ADD_CONSOLE_COMMAND("editor", "open_project", [this](const GEVector<GEString>& args) {
			OpenProject(args[0].ToPath());
			}, "editor.open_project <directory>");
		GE_ADD_CONSOLE_COMMAND("editor", "save_project", [this](const GEVector<GEString>& args) {
			SaveProject();
			});

		id = ge::renderer::Renderer3D::GetImGuiTexture(_scene->GetSceneRenderer()->GetOffscreenFramebuffer()->GetColorAttachmentTexture(0));
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

	virtual void OnImGuiRender() override {
		ImGui::Begin("Viewport");
		ImGui::Image(id, ImVec2(300, 300));

		if (ImGui::MenuItem("Import Model...")) {
			GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.showPanel E_mI");
		}
		ImGui::End();
	}
private:
	void NewProject(const GEString& name, const std::filesystem::path& dir) { ge::Project::New(name, dir); }
	void OpenProject(const std::filesystem::path& path) {
		if (ge::Project::Load(path)) {
			_scene = ge::mem::Ref<ge::Scene>::Create("New Scene");
			_scene->CreateSceneRenderer();
		};
	}
	void SaveProject() {
		ge::Project::SaveActive();
	}
private:
	ge::mem::Ref<ge::Scene> _scene;
	ge::mem::Ref<ge::renderer::FreeCamera> _camera;
	ImTextureID id;

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
	Application* CreateApplication(const ApplicationCommandLineArgs& args) {
		ApplicationSpecification appSpecs{};
		appSpecs.title = GEString("Glass Editor");
		appSpecs.mode = ApplicationMode::Editor;
		appSpecs.width = 1280;
		appSpecs.height = 720;
		appSpecs.commandLineArgs = args;
		return new EditorApp(appSpecs);
	}
}