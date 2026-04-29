#pragma once
#include <GlassEngine/GE.h>

namespace ge::editor {
	class EditorLayer : public Layer {
	public:
		EditorLayer() : Layer("LAYER_Editor") {}
		virtual void OnAttach() override {
			_camera = mem::Ref<renderer::FreeCamera>::Create();
			OpenProject("projects/sandbox/sandbox.geproj");

			GetPanelManager().RegisterPanel<editor::Console>("E_c");
			GetPanelManager().RegisterPanel<editor::EditorECSDebugPanel>("E_ecs", _scene.Get());
			GetPanelManager().RegisterPanel<editor::EditorRendererDebugPanel>("E_r");
			GetPanelManager().RegisterPanel<editor::EditorMemoryDebugPanel>("E_m");
			GetPanelManager().RegisterPanel<editor::ModelImportPanel>("E_mI");

			GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.showPanel E_c");
			GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.showPanel E_ecs");
			GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.showPanel E_r");

			GE_ADD_CONSOLE_COMMAND("editor", "save_scene", [this](const GEVector<GEString>& args) {
				SceneSerializer serializer(_scene);
				serializer.Serialize(args[0]);
				}, "editor.save_scene <path>");

			GE_ADD_CONSOLE_COMMAND("editor", "load_scene", [this](const GEVector<GEString>& args) {
				SceneSerializer serializer(_scene);
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

			id = renderer::Renderer3D::GetImGuiTexture(_scene->GetSceneRenderer()->GetOffscreenFramebuffer()->GetColorAttachmentTexture(0));

			{
				auto marioModel = AssetManager::GetOrImportAsset<renderer::StaticMesh>("Resources/DamagedHelmet/DamagedHelmet.gltf");
				entity = _scene->CreateEntity("Mario");
				auto& smc = entity->AddComponent<StaticMeshComponent>();
				smc.meshHandle = marioModel->_assetHandle;
			}
		}

		virtual void OnDetach() override {}
		virtual void OnUpdate(float deltaTime) override {
			_camera->Update(deltaTime);

			_scene->OnEditorUpdate(deltaTime, _camera);

			if (ge::Engine::Get().GetInputManager().IsKeyJustPressed(key::Tab)) {
				_cursor = !_cursor;
				_camera->SetProccessingMouse(!_cursor);
				_camera->SetFirstMouse();
				ge::Engine::Get().GetApplicationWindow().SetCursor(_cursor);
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
		void NewProject(const GEString& name, const std::filesystem::path& dir) { Project::New(name, dir); }
		void OpenProject(const std::filesystem::path& path) {
			if (Project::Load(path)) {
				_scene = mem::Ref<Scene>::Create("New Scene");
				_scene->CreateSceneRenderer();
			};
		}
		void SaveProject() {
			Project::SaveActive();
		}
	private:
		mem::Ref<Scene> _scene;
		mem::Ref<renderer::FreeCamera> _camera;
		ImTextureID id;

		bool _cursor = false;
		Entity* entity;
	};
}