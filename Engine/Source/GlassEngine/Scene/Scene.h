#pragma once
#include <entt/entt.hpp>
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Memory/Memory.h"
#include "GlassEngine/GUI/GUIECSDebugPanel.h"
#include "Entity.h"

namespace ge {
	class Scene : public mem::RefCounted {
	public:
		Scene(const GEString& name);
		~Scene();

		Entity* CreateEntity(const GEString& name = "Entity");
		Entity* CreateEntityWithID(EntityID id, const GEString& name = "Entity");
		void DestroyEntity(Entity* entity);
		void Clear();

		Entity* GetEntityByID(EntityID id);
		Entity* GetEntityByName(std::string_view name);

		void OnRuntimeStart();
		void OnRuntimeStop();
		void OnRuntimeUpdate(float DeltaTime);
		void OnEditorUpdate(float DeltaTime);

		const GEString& GetName() const { return _name; }
		entt::registry& GetRegistry() { return _registry; }
		GEUnorderedMap<EntityID, mem::Ref<Entity>>& GetEntities() { return _entities; }
	private:
		GEString _name;
		gui::GUIECSDebugPanel _ecsDebugPanel{ this };
		entt::registry _registry;
		GEUnorderedMap<EntityID, mem::Ref<Entity>> _entities;
	};
}
#include "EntityFunctions.h"