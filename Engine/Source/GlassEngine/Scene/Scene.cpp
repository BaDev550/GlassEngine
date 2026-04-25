#include "gepch.h"
#include "Scene.h"

#include "Components/BaseComponents.h"

namespace ge {
	Scene::Scene(const GEString& name) : _name(name) {
		GE_ADD_CONSOLE_COMMAND("scene", "clear_scene", [this](const GEVector<GEString>& args) { Clear(); });

		_sceneRenderer = mem::Ref<renderer::SceneRenderer>::Create(this);
	}
	Scene::~Scene(){ Clear(); }

	void Scene::Clear() {
		 _registry.clear();
		 _entities.clear();
	}

	Entity* Scene::CreateEntity(const GEString& name) { return CreateEntityWithID(EntityID(), name); }
	Entity* Scene::CreateEntityWithID(EntityID id, const GEString& name)
	{
		mem::Ref<Entity> entity = mem::Ref<Entity>::Create(_registry.create(), this);
		entity->AddComponent<IdentityComponent>(name, id);
		entity->AddComponent<TransformComponent>();
		_entities[id] = entity;
		return entity.Get();
	}

	void Scene::DestroyEntity(Entity* entity)
	{
		if (!entity) return;
		_registry.destroy(*entity);
		_entities.erase(entity->GetEntityID());
	}

	Entity* Scene::GetEntityByID(EntityID id)
	{
		auto it = _entities.find(id);
		return it != _entities.end() ? it->second.Get() : nullptr;
	}

	Entity* Scene::GetEntityByName(std::string_view name)
	{
		for (auto& [id, entity] : _entities) {
			if (entity->HasComponent<IdentityComponent>()) {
				auto& identity = entity->GetComponent<IdentityComponent>();
				if (identity.name == name)
					return entity.Get();
			}
		}
		return nullptr;
	}

	void Scene::OnRuntimeStart()
	{
	}

	void Scene::OnRuntimeStop()
	{
	}

	void Scene::OnRuntimeUpdate(float DeltaTime)
	{
	}

	void Scene::OnEditorUpdate(float DeltaTime, mem::Ref<renderer::Camera>& cam)
	{
		_sceneRenderer->DrawScene(cam);
	}
}