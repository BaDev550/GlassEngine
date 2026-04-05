#include <iostream>
#include <GlassEngine/Core/EntryPoint.h>
#include <GlassEngine/Core/Application.h>
#include <GlassEngine/Layers/Layer.h>
#include <GlassEngine/Memory/Ref.h>
#include <GlassEngine/Memory/PoolAllocator.h>
#include <GlassEngine/Core/Core.h>

struct Component {
	float pos[3]{};
	float ros[3]{};
};
#define MAX_COMPONENT_COUNT_PER_ENTITY 30
#define MAX_ENTITIES 1000

#define MAX_COMPONENTS MAX_ENTITIES * MAX_COMPONENT_COUNT_PER_ENTITY

class EditorLayer : public ge::Layer {
public:
	EditorLayer() : ge::Layer("EditorLayer") {
		GE_APPLICATOIN_INFO("EditorLayer created!");
	}
	virtual void OnAttach() override {
		GE_APPLICATOIN_INFO("EditorLayer attached!");

		GE_PROFILE_SCOPE("GEA_MallocTest"); {
#define USE_POOL
#ifdef USE_POOL
			GEVector<Component*> _components;
			_poolAllocator = new ge::mem::PoolAllocator(sizeof(Component), MAX_COMPONENTS, "componentPool");
			for (int i = 0; i < MAX_COMPONENTS + 1; i++) {
				if (Component* comp = static_cast<Component*>(_poolAllocator->Allocate())) {
					_components.push_back(comp);
					_components[i]->pos[0] = 1.0f;
					_components[i]->pos[1] = 4.0f + i;
					_components[i]->pos[2] = 2.0f;
					GE_APPLICATOIN_INFO("x:{}y:{}z:{}", _components[i]->pos[0], _components[i]->pos[1], _components[i]->pos[2]);
				}
			}

			for (auto& comp : _components)
				_poolAllocator->Free(comp);
		}
#endif
#ifdef USE_MALLOC
		GEVector<Component*> _components;
		for (int i = 0; i < MAX_COMPONENTS + 1; i++) {
			_components.push_back(CastChecked<Component>(ge::mem::allocFuncs::GE_Allocate(sizeof(Component))));
			_components[i]->pos[0] = 1.0f;
			_components[i]->pos[1] = 4.0f + i;
			_components[i]->pos[2] = 2.0f;
			GE_APPLICATOIN_INFO("x:{}y:{}z:{}", _components[i]->pos[0], _components[i]->pos[1], _components[i]->pos[2]);
		}
		for (auto& comp : _components)
			ge::mem::allocFuncs::GE_Free(comp, sizeof(Component));
		}
#endif
	}

	virtual void OnDetach() override {
		GE_APPLICATOIN_INFO("EditorLayer detached!");
		delete _poolAllocator;
		_poolAllocator = nullptr;
	}
	virtual void OnUpdate(float deltaTime) override {}
	ge::mem::PoolAllocator* _poolAllocator;
};

class EditorApp : public ge::Application {
public:
	EditorApp(const ge::ApplicationSpecification& createInfo) : ge::Application(createInfo) {
		GE_APPLICATOIN_INFO("EditorApp created!");
		PushLayer(new EditorLayer());
	}
};

namespace ge {
	Application* CreateApplication(const ApplicationSpecification& createInfo) {
		return new EditorApp(createInfo);
	}
}