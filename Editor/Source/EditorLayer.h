#pragma once
#include <GlassEngine/GE.h>

namespace ge::editor {
	enum class EditorMode : uint8_t {
		Editor,
		Simulate,
		Playing
	};

	class EditorLayer : public Layer {
	public:
		EditorLayer() : Layer("LAYER_Editor") {}
		virtual void OnAttach() override;
		virtual void OnDetach() override {}
		virtual void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;
	private:
		void NewScene();
		void LoadScene(const std::filesystem::path& path);
		void SaveScene();

		void NewProject(const GEString& name, const std::filesystem::path& dir);
		void OpenProject(const std::filesystem::path& path);
		void SaveProject();

		virtual void RegisterConsoleCommands() override;
		virtual void RegisterLayerPanels() override;
	private:
		void BeginDockspace();
		void EndDockspace();

		mem::Ref<Scene> _activeScene;
		mem::Ref<Scene> _editorScene;
		std::filesystem::path _activeScenePath;

		mem::Ref<renderer::FreeCamera> _camera;

		ImTextureID _viewportTextureId;
		ImTextureID _gBufferNormalTextureId;
		ImTextureID _gBufferAlbedoTextureId;
		ImTextureID _gBufferRoughnessTextureId;
		ImVec2 _viewportSize;

		bool _consoleEnabled = false;
		bool _cursor = false;
		Entity* entity;
		editor::ContentBrowserPanel* _contentBrowserPanel;
		editor::SceneHierarchyPanel* _sceneHierarchyPanel;
	};
}