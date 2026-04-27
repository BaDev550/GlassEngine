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
#include <GlassEngine/Scene/SceneSerializer.h>
#include <GlassEngine/Editor/EditorConsole.h>
#include <GlassEngine/Editor/EditorECSDebugPanel.h>
#include <GlassEngine/Editor/EditorRendererDebugPanel.h>

#include <imgui.h>

class EditorLayer : public ge::Layer {
public:
	EditorLayer() : ge::Layer("LAYER_Editor") {}
	virtual void OnAttach() override {
		_scene = ge::mem::Ref<ge::Scene>::Create("Test Scene");
		_camera = ge::mem::Ref<ge::renderer::FreeCamera>::Create();

		GetPanelManager().RegisterPanel<ge::editor::Console>("E_c");
		GetPanelManager().RegisterPanel<ge::editor::EditorECSDebugPanel>("E_ecs", _scene.Get());
		GetPanelManager().RegisterPanel<ge::editor::EditorRendererDebugPanel>("E_r");

		GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.showPanel E_c");
		GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.showPanel E_ecs");
		GE_EXECUTE_CONSOLE_COMMAND("LAYER_Editor.showPanel E_r");
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

		GE_ADD_CONSOLE_COMMAND("editor", "save_scene", [this](const GEVector<GEString>& args) {
			ge::SceneSerializer serializer(_scene);
			serializer.Serialize(args[0]);
			}, "editor.save_scene <path>");

		GE_ADD_CONSOLE_COMMAND("editor", "load_scene", [this](const GEVector<GEString>& args) { 
			ge::SceneSerializer serializer(_scene);
			serializer.Deserialize(args[0]);
			}, "editor.load_scene <path>");

		ge::renderer::FramebufferSpec fspec{};
		fspec.attachments = { ge::renderer::FramebufferAttachment{true}, ge::renderer::FramebufferAttachment{ge::renderer::ImageFormat::D32S8} };
		fspec.attachments[1].clearValue = { 1.f, 0 };
		fspec.width = ge::Application::Get()->GetWindow().GetWidth();
		fspec.height = ge::Application::Get()->GetWindow().GetHeight();
		_framebuffer = ge::renderer::Framebuffer::Create(fspec);
		_renderPass = ge::renderer::RenderPass::Create(_framebuffer, "GUI_RENDER_PASS");

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
		_renderPass->Begin();
		ImGui::Begin("Test");
		ImGui::Text("Test");
		ImGui::Image(id, ImVec2(800, 800));
		ImGui::End();
		_renderPass->End();
	}
private:
	ge::mem::Ref<ge::Scene> _scene;
	ge::mem::Ref<ge::renderer::FreeCamera> _camera;
	ge::mem::Ref<ge::renderer::Framebuffer> _framebuffer;
	ge::mem::Ref<ge::renderer::RenderPass> _renderPass;
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
	Application* CreateApplication(const ApplicationSpecification& createInfo) {
		return new EditorApp(createInfo);
	}
}