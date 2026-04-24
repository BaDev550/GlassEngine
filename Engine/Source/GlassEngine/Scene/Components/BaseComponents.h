#pragma once
#include <glm/glm.hpp>
#include "GlassEngine/Core/Core.h"

namespace ge {
#define GE_INVALID_ENTITY_ID 0
	using EntityID = UUID;

	struct IdentityComponent {
		GEString name;
		EntityID id;

		IdentityComponent(const GEString& name = "UNDEFINED_ENTITY_NAME", EntityID id = 0) : name(name), id(id) {}
		IdentityComponent(const IdentityComponent&) = default;
	};

	struct TransformComponent {
		glm::vec3 position{ 0.0f };
		glm::vec3 rotation{ 0.0f };
		glm::vec3 scale{ 1.0f };

		TransformComponent(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& rotation = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f)) : position(position), rotation(rotation), scale(scale) {}
		TransformComponent(const TransformComponent&) = default;
	};
}