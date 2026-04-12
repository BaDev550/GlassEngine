#include "gepch.h"
#include "Application.h"
#include "GlassEngine/Renderer/Renderer.h"
#include "GlassEngine/Renderer/Texture.h"
#include "GlassEngine/Asset/AssetManager.h"
#include <stdexcept>
#include <iostream>

namespace ge {
	Application* Application::_instance = nullptr;
	Application::Application(const ApplicationSpecification& specs) : _specs(specs) {
		if (_instance)
			throw std::runtime_error("Application already exists!");
		_instance = this;

		Logger::Init();
		_window = mem::CreateScope<Window>(WindowSpecification({ _specs.title, _specs.width, _specs.height }));
		_threadManager = mem::CreateScope<ThreadManager>(1); // 1 for asset manager add more later for application
		renderer::Renderer3D::Init();
		_editorAssetManager = mem::CreateScope<EditorAssetManager>();
		_runtimeAssetManager = mem::CreateScope<RuntimeAssetManager>("Assets.pak");
		_defaultAssetManager = (_specs.mode == ApplicationMode::Editor) ? 
			CastChecked<AssetManager>(_editorAssetManager.get()) : 
			CastChecked<AssetManager>(_runtimeAssetManager.get());

		ImportAssetData assetData{};
		ge::renderer::TextureSpecification textureSpecs{};
		textureSpecs.filter = ge::renderer::ImageFilter::Linear;
		textureSpecs.format = ge::renderer::ImageFormat::RGBA8;
		assetData.textureSpecs = &textureSpecs;
		GetEditorAssetManager().ImportAssetAsync(assetData, [this](AssetHandle iconHandle) {
			auto iconAsset = GetEditorAssetManager().GetAsset(iconHandle).Cast<renderer::Texture2D>();
			_window->SetIcon(iconAsset->GetData(), iconAsset->GetWidth(), iconAsset->GetHeight());
			}, "Resouces/icon-512.png");
	}

	Application::~Application() {
		renderer::Renderer3D::Destroy();
		Logger::Destroy();
	}

	void Application::Run() {
		while (!_window->ShoudClose() && !_forceClose) {
			_window->PollEvents();

			for (auto& layer : _layerStack)
				layer->OnUpdate(0.0f);
		}
		_window->GetRenderContext().Wait();
	}
}