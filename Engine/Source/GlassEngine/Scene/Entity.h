#pragma once
#include <entt/entt.hpp>
#include "Components/BaseComponents.h"
#include "GlassEngine/Utilities/UUID.h"

namespace ge {
	class Scene;
	class Entity : public mem::RefCounted {
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene) : _handle(handle), _scene(scene) {}
		Entity(const Entity& other) = default;

		template<typename T>
		bool HasComponent() const;
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args);
		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args);
		template<typename T>
		T& GetComponent();
		template<typename T>
		const T& GetComponent() const;
		template<typename T>
		void RemoveComponent();

		const entt::entity GetEntityHandle() const { return _handle; }
		const EntityID GetEntityID() const { return GetComponent<IdentityComponent>().id; }
		const GEString& GetName() const { return GetComponent<IdentityComponent>().name; }

		operator entt::entity() const { return _handle; }
		operator bool() const { return _handle != entt::null; }
		operator uint64_t() const { return (uint64_t)_handle; }
		bool operator==(const Entity& other) const { return  (_handle == other._handle && _scene == other._scene); }
		bool operator!=(const Entity& other) const { return !(*this == other); }
	private:
		entt::entity _handle = entt::null;
		Scene* _scene = nullptr;
	};
}