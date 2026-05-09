#pragma once
#include <glm/glm.hpp>

namespace ge {
#define GE_MAX_POINT_LIGHT_COUNT 1024
#define GE_MAX_SPOT_LIGHT_COUNT 1024
	struct alignas(16) PointLight {
		glm::vec4 position;
		glm::u8vec4 color = { 255, 255, 255, 255 };
		float intensity = 100.0f;
		float radius = 100.0f;
		float _pad0;
	};

	struct alignas(16) SpotLight {
		glm::vec4 position;
		glm::vec4 direction;
		glm::u8vec4 color;
		float intensity;
		float radius;
		float _pad0;
	};

	struct alignas(16) DirectionalLight {
		glm::vec4 direction;
		glm::u8vec4 color = { 255, 255, 255, 255 };
		float intensity = 1.0f;
		float _pad0;
		float _pad1;
	};
}