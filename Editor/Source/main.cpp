#include <iostream>
#include <GlassEngine/Core/EntryPoint.h>
#include <GlassEngine/Core/Application.h>
#include <GlassEngine/Layers/Layer.h>
#include <GlassEngine/Core/Memory.h>
#include <GlassEngine/Core/Core.h>
#include <GlassEngine/Renderer/Texture.h>

class EditorLayer : public ge::Layer {
public:
	EditorLayer() : ge::Layer("EditorLayer") {}
	virtual void OnAttach() override {}
	virtual void OnDetach() override {}
	virtual void OnUpdate(float deltaTime) override {}
};

class EditorApp : public ge::Application {
public:
	EditorApp(const ge::ApplicationSpecification& createInfo) : ge::Application(createInfo) {
		GE_APPLICATOIN_INFO("EditorApp created!");
		PushLayer(new EditorLayer());

		//ge::ImportAssetData assetData{};
		//ge::renderer::TextureSpecification specs{};
		//specs.filter = ge::renderer::ImageFilter::Linear;
		//specs.format = ge::renderer::ImageFormat::RGBA8;
		//assetData.textureSpecs = &specs;
		//ge::AssetHandle texture = GetAssetManager().ImportAsset(assetData, "Resouces/Texture_01.png", "Assets/Textures/Texture_01");
		//auto asset = GetAssetManager().GetAsset(texture).Cast<ge::renderer::Texture2D>();

		//GetAssetManager().CompileIntoPakFile("Assets.pak");
	}
};

namespace ge {
	Application* CreateApplication(const ApplicationSpecification& createInfo) {
		return new EditorApp(createInfo);
	}
}