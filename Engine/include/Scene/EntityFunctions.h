#pragma once

namespace ge {
	template<typename T>
	bool Entity::HasComponent() const {
		return _scene->GetRegistry().any_of<T>(_handle);
	}

	template<typename T, typename... Args>
	T& Entity::AddComponent(Args&&... args) {
		if (HasComponent<T>()) {
			return GetComponent<T>();
		}
		T& component = _scene->GetRegistry().emplace<T>(_handle, std::forward<Args>(args)...);
		return component;
	}

	template<typename T, typename... Args>
	T& Entity::AddOrReplaceComponent(Args&&... args) {
		T& component = _scene->GetRegistry().emplace_or_replace<T>(_handle, std::forward<Args>(args)...);
		return component;
	}

	template<typename T>
	T& Entity::GetComponent() {
		GE_ASSERT(HasComponent<T>(), "Entity doesnt have this component");
		return _scene->GetRegistry().get<T>(_handle);
	}

	template<typename T>
	const T& Entity::GetComponent() const {
		GE_ASSERT(HasComponent<T>(), "Entity doesnt have this component");
		return _scene->GetRegistry().get<T>(_handle);
	}

	template<typename T>
	void Entity::RemoveComponent() {
		GE_ASSERT(HasComponent<T>(), "Entity doesnt have this component");
		_scene->GetRegistry().remove<T>(_handle);
	}
}