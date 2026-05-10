#include "Scene/Scene.h"

#include "Scene/SceneSerializer.h"
#include "Scene/Components/BaseComponents.h"

namespace ge {
	Scene::Scene(const GEString& name) : _name(name) {}
	Scene::~Scene(){ Clear(); }

	void Scene::Clear() {
		 _registry.clear();
		 _entities.clear();
	}

	Entity& Scene::CreateEntity(const GEString& name) { return CreateEntityWithID(EntityID(), name); }
	Entity& Scene::CreateEntityWithID(EntityID id, const GEString& name)
	{
		_entities[id] = Entity(_registry.create(), this);
		_entities[id].AddComponent<IdentityComponent>(name, id);
		_entities[id].AddComponent<TransformComponent>();
		return _entities[id];
	}

	void Scene::DestroyEntity(Entity* entity)
	{
		if (!entity) return;
		EntityID id = entity->GetEntityID();
		_registry.destroy(*entity);
		_entities.erase(id);
	}

	Entity Scene::GetEntityByID(EntityID id)
	{
		auto it = _entities.find(id);
		return it != _entities.end() ? it->second : Entity();
	}

	Entity Scene::GetEntityByName(std::string_view name)
	{
		for (auto& [id, entity] : _entities) {
			if (entity.HasComponent<IdentityComponent>()) {
				auto& identity = entity.GetComponent<IdentityComponent>();
				if (identity.name == name)
					return entity;
			}
		}
		return Entity();
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

	void Scene::OnEditorUpdate(float DeltaTime)
	{
	}
}