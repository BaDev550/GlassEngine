#include "EditorLayer.h"

#include <imgui_impl_vulkan.h>

namespace ge::editor {
	void EditorLayer::OnAttach()
	{
		Layer::OnAttach();

		// TEMP
		auto projectPath = GE_ENGINE_DIR.parent_path() / "sandbox/sandbox.geproj";

		OpenProject(projectPath);
		RegisterConsoleCommands();

		_camera = mem::Ref<renderer::FreeCamera>::Create();
		_viewportTextureId = renderer::Renderer3D::GetImGuiTexture("EDITOR_VIEWPORT_ID", _activeScene->GetSceneRenderer()->GetOffscreenFramebuffer()->GetColorAttachmentTexture(0));
		_gBufferNormalTextureId = renderer::Renderer3D::GetImGuiTexture("EDITOR_GBUFF_NORM_ID", _activeScene->GetSceneRenderer()->GetGBufferFramebuffer()->GetColorAttachmentTexture(0));
		_gBufferAlbedoTextureId = renderer::Renderer3D::GetImGuiTexture("EDITOR_GBUFF_ALB_ID", _activeScene->GetSceneRenderer()->GetGBufferFramebuffer()->GetColorAttachmentTexture(1));
		_contentBrowserPanel = CastChecked<editor::ContentBrowserPanel>(GetPanelManager().ShowPanel("contentBrowser"));
		_contentBrowserPanel->Init();
		_sceneHierarchyPanel = CastChecked<editor::SceneHierarchyPanel>(GetPanelManager().ShowPanel("sceneHierarchy"));
		_sceneHierarchyPanel->SetContext(_activeScene);
	}

	void EditorLayer::OnUpdate(float deltaTime)
	{
		_camera->Update(deltaTime);

		auto spec = _activeScene->GetSceneRenderer()->GetOffscreenFramebuffer()->GetSpecification();
		if (_viewportSize.x > 0.0f && _viewportSize.y > 0.0f && (spec.width != _viewportSize.x || spec.height != _viewportSize.y)) {
			Engine::Get().GetApplicationWindow().GetRenderContext().Wait();

			_activeScene->GetSceneRenderer()->Resize(_viewportSize.x, _viewportSize.y);
			_camera->SetAspectRatio(((float)_viewportSize.x / (float)_viewportSize.y));
			_viewportTextureId = renderer::Renderer3D::GetImGuiTexture("EDITOR_VIEWPORT_ID", _activeScene->GetSceneRenderer()->GetOffscreenFramebuffer()->GetColorAttachmentTexture(0));
			_gBufferNormalTextureId = renderer::Renderer3D::GetImGuiTexture("EDITOR_GBUFF_NORM_ID", _activeScene->GetSceneRenderer()->GetGBufferFramebuffer()->GetColorAttachmentTexture(0));
			_gBufferAlbedoTextureId = renderer::Renderer3D::GetImGuiTexture("EDITOR_GBUFF_ALB_ID", _activeScene->GetSceneRenderer()->GetGBufferFramebuffer()->GetColorAttachmentTexture(1));
		}

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
		_viewportSize = ImGui::GetContentRegionAvail();
		ImGui::Image(_viewportTextureId, _viewportSize);

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				AssetHandle droppedHandle = *(const AssetHandle*)payload->Data;
				auto droppedMTD = AssetManager::GetMetadata(droppedHandle);
				
				if (droppedMTD.type == AssetType::StaticMesh) {
					mem::Ref<renderer::StaticMesh> mesh = AssetManager::GetAsset<renderer::StaticMesh>(droppedHandle);
					if (mesh) {
						auto meshEntity = _activeScene->CreateEntity(TEXT(droppedMTD.path.stem().string()));
						auto& meshComponent = meshEntity->AddComponent<StaticMeshComponent>();
						meshComponent.meshHandle = mesh->_assetHandle;
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::End();

		ImGui::Begin("GBuffer");
		ImGui::Image(_gBufferNormalTextureId, ImVec2(300,300));
		ImGui::Image(_gBufferAlbedoTextureId, ImVec2(300, 300));
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
			_activeScene->Clear();
			_activeScene = std::move(tempScene);
			_activeScene->CreateSceneRenderer();
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
			auto defaultScenePath = Project::GetActive()->GetDefaultScenePath();
			_activeScene = mem::Ref<Scene>::Create(defaultScenePath.stem().string());

			SceneSerializer serializer(_activeScene);
			serializer.Deserialize(defaultScenePath.string());
			_activeScenePath = defaultScenePath;
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