#include "EditorLayer.h"

namespace ge::editor {
	void EditorLayer::OnAttach()
	{
		Layer::OnAttach();

		// TEMP
		auto projectPath = GE_ENGINE_DIR.parent_path() / "sandbox/sandbox.geproj";

		OpenProject(projectPath);
		RegisterConsoleCommands();

		_camera = mem::Ref<renderer::FreeCamera>::Create();
		_viewportTextureId = renderer::Renderer3D::GetImGuiTexture(_activeScene->GetSceneRenderer()->GetOffscreenFramebuffer()->GetColorAttachmentTexture(0));
		_contentBrowserPanel = CastChecked<editor::ContentBrowserPanel>(GetPanelManager().ShowPanel("contentBrowser"));
		_contentBrowserPanel->Init();
		_sceneHierarchyPanel = CastChecked<editor::SceneHierarchyPanel>(GetPanelManager().ShowPanel("sceneHierarchy"));
		_sceneHierarchyPanel->SetContext(_activeScene);

		{
			auto marioModel = AssetManager::GetOrImportAsset<renderer::StaticMesh>(projectPath.parent_path() / "Assets/Model/DamagedHelmet.gltf");
			AssetManager::GetOrImportAsset<renderer::StaticMesh>(projectPath.parent_path() / "Assets/mario_2/mario_2.obj");
			entity = _activeScene->CreateEntity("Mario");
			auto& smc = entity->AddComponent<StaticMeshComponent>();
			smc.meshHandle = marioModel->_assetHandle;
		}
	}

	void EditorLayer::OnUpdate(float deltaTime)
	{
		_camera->Update(deltaTime);

		_activeScene->OnEditorUpdate(deltaTime, _camera);

		if (Engine::Get().GetInputManager().IsKeyJustPressed(key::Tab)) {
			_cursor = !_cursor;
			_camera->SetProccessingMouse(!_cursor);
			_camera->SetFirstMouse();
			ge::Engine::Get().GetApplicationWindow().SetCursor(_cursor);
		}

		if (Engine::Get().GetInputManager().IsKeyPressed(key::LeftAlt) && Engine::Get().GetInputManager().IsKeyJustPressed(key::C)) {
			_consoleEnabled = !_consoleEnabled;
			if (_consoleEnabled) GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.showPanel console");
			else GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.hidePanel console");
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		BeginDockspace();
		Layer::OnImGuiRender();

		ImGui::Begin("Viewport");
		ImVec2 viewportSize = ImGui::GetWindowSize();
		ImGui::Image(_viewportTextureId, viewportSize);
		ImGui::End();
		EndDockspace();
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
			_sceneHierarchyPanel->SetContext(_activeScene);
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
		GetPanelManager().RegisterPanel<editor::EditorConsole>("console");
		GetPanelManager().RegisterPanel<editor::EditorRendererDebugPanel>("render");
		GetPanelManager().RegisterPanel<editor::EditorMemoryDebugPanel>("mem");
		GetPanelManager().RegisterPanel<editor::ModelImportPanel>("modelImporter");
		GetPanelManager().RegisterPanel<editor::ContentBrowserPanel>("contentBrowser");
		GetPanelManager().RegisterPanel<editor::SceneHierarchyPanel>("sceneHierarchy");
	}

	void EditorLayer::BeginDockspace()
	{
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Glass Engine Editor Dockspace", nullptr, window_flags);
		ImGui::PopStyleVar();
		ImGui::PopStyleVar(2);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("GlassEngineDockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
	}

	void EditorLayer::EndDockspace() {
		ImGui::End();
	}
}