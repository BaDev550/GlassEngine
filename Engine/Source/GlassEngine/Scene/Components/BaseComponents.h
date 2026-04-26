#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

		glm::mat4 Mat4() const {
			return glm::translate(glm::mat4(1.0f), position)
				* glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
				* glm::scale(glm::mat4(1.0f), scale);
		}

		TransformComponent(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& rotation = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f)) : position(position), rotation(rotation), scale(scale) {}
		TransformComponent(const TransformComponent&) = default;
	};

	struct StaticMeshComponent {
		AssetHandle meshHandle;
		bool isVisible = true;
		bool calculateLOD = true;
		uint32_t lodLevel = 0;
		mem::Ref<renderer::MaterialTable> materialTable = nullptr;

		StaticMeshComponent(AssetHandle handle = GE_INVALID_ASSET_HANDLE) : meshHandle(handle) {}
		StaticMeshComponent(const StaticMeshComponent&) = default;
	};
}