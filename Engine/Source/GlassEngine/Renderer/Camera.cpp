#include "gepch.h"
#include "Camera.h"

#include "GlassEngine/Core/Application.h"

namespace ge::renderer {
	void Camera::Orbit(float pitchOffset, float yawOffset)
	{
		AddPitch(pitchOffset);
		AddYaw(yawOffset);
	}

	void Camera::CalculateCameraMatrixes()
	{
		_forward.x = cosf(_direction.x) * sinf(_direction.y);
		_forward.y = sinf(_direction.x);
		_forward.z = -cosf(_direction.x) * cosf(_direction.y);
		_forward = glm::normalize(_forward);
		
		auto& window = Application::Get()->GetWindow();
		float width = (float)window.GetWidth();
		float height = (float)window.GetHeight();
		float aspect = width / height;

		glm::vec3 worldUp = glm::vec3(0.0f, -1.0f, 0.0f);
		if (glm::abs(glm::dot(_forward, worldUp)) > 0.999f) { _right = glm::normalize(glm::cross(glm::vec3(0.0f, 0.0f, -1.0f), _forward)); }
		else { _right = glm::normalize(glm::cross(_forward, worldUp)); }

		_viewMatrix = glm::lookAt(_position, _position + _forward, _up);
		_projectionMatrix = glm::perspective(glm::radians(45.0f), aspect, _nearPlane, _farPlane);
	}

	void Camera::AddPitch(float value)
	{
		_direction.x += value;
	}

	void Camera::AddYaw(float value)
	{
		_direction.y += value;
	}
}