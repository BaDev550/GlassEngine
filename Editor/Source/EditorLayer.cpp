#include "EditorLayer.h"

namespace ge::editor {
	void EditorLayer::OnAttach()
	{
		Layer::OnAttach();
		GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.showPanel console");
		GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.showPanel render");

		OpenProject("projects/sandbox/sandbox.geproj");

		_camera = mem::Ref<renderer::FreeCamera>::Create();
		_viewportTextureId = renderer::Renderer3D::GetImGuiTexture(_activeScene->GetSceneRenderer()->GetOffscreenFramebuffer()->GetColorAttachmentTexture(0));

		{
			auto marioModel = AssetManager::GetOrImportAsset<renderer::StaticMesh>("Resources/DamagedHelmet/DamagedHelmet.gltf");
			entity = _activeScene->CreateEntity("Mario");
			auto& smc = entity->AddComponent<StaticMeshComponent>();
			smc.meshHandle = marioModel->_assetHandle;
		}
	}

	void EditorLayer::OnUpdate(float deltaTime)
	{
		_camera->Update(deltaTime);

		_activeScene->OnEditorUpdate(deltaTime, _camera);

		if (ge::Engine::Get().GetInputManager().IsKeyJustPressed(key::Tab)) {
			_cursor = !_cursor;
			_camera->SetProccessingMouse(!_cursor);
			_camera->SetFirstMouse();
			ge::Engine::Get().GetApplicationWindow().SetCursor(_cursor);
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		ImGui::Begin("Viewport");
		ImVec2 viewportSize = ImGui::GetWindowSize();
		ImGui::Image(_viewportTextureId, viewportSize);
		ImGui::End();
	}

	void EditorLayer::NewScene()
	{
		_activeScene = mem::Ref<Scene>::Create("NewScene"); // TODO (badev): take a name in func to lazy to do it rn
	}

	void EditorLayer::LoadScene(const std::filesystem::path& path) {
		mem::Ref<Scene> tempScene = mem::Ref<Scene>::Create(path.stem().string());
		SceneSerializer serializer(tempScene);
		if (serializer.Deserialize(path.string())) {
			_activeScenePath = path;
			_activeScene->Clear();
			_activeScene = std::move(tempScene);
		}
	}

	void EditorLayer::SaveScene() {
		SceneSerializer serializer(_activeScene);
		serializer.Serialize(_activeScenePath.string());
	}

	void EditorLayer::NewProject(const GEString& name, const std::filesystem::path& dir) { Project::New(name, dir); }
	void EditorLayer::OpenProject(const std::filesystem::path& path) {
		if (Project::Load(path)) {
			_activeScene = mem::Ref<Scene>::Create("New Scene");
			_editorScene = _activeScene;
			_activeScene->CreateSceneRenderer();
		};
	}
	void EditorLayer::SaveProject() {
		Project::SaveActive();
	}

	void EditorLayer::RegisterConsoleCommands()
	{
		GE_ADD_CONSOLE_COMMAND("editor", "save_scene", [this](const GEVector<GEString>& args) { SaveScene(); });
		GE_ADD_CONSOLE_COMMAND("editor", "load_scene", [this](const GEVector<GEString>& args) { LoadScene(args[0].ToPath()); }, "editor.load_scene <path>");
		GE_ADD_CONSOLE_COMMAND("editor", "create_project", [this](const GEVector<GEString>& args) { NewProject(args[0], args[1].ToPath()); }, "editor.create_project <name> <directory>");
		GE_ADD_CONSOLE_COMMAND("editor", "open_project", [this](const GEVector<GEString>& args) { OpenProject(args[0].ToPath()); }, "editor.open_project <directory>");
		GE_ADD_CONSOLE_COMMAND("editor", "save_project", [this](const GEVector<GEString>& args) { SaveProject(); });
	}

	void EditorLayer::RegisterLayerPanels() {
		GetPanelManager().RegisterPanel<editor::Console>("console");
		GetPanelManager().RegisterPanel<editor::EditorECSDebugPanel>("ecs", _editorScene);
		GetPanelManager().RegisterPanel<editor::EditorRendererDebugPanel>("render");
		GetPanelManager().RegisterPanel<editor::EditorMemoryDebugPanel>("mem");
		GetPanelManager().RegisterPanel<editor::ModelImportPanel>("modelImporter");
	}
}