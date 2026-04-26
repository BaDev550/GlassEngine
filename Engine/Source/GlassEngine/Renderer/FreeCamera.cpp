#include "gepch.h"
#include "FreeCamera.h"
#include "GlassEngine/Core/Input.h"

namespace ge::renderer {
	void FreeCamera::Update(float dt)
	{
		if (_proccessingMouse) {
			glm::vec3 forward = glm::normalize(_forward);
			glm::vec3 right = glm::normalize(_right);
			glm::vec3 position = _position;

			float speed = 5.0f;
			if (Input::IsKeyPressed(key::LeftShift))
				speed *= 2.5f;

			if (Input::IsKeyPressed(key::W))
				position += forward * speed * dt;
			if (Input::IsKeyPressed(key::S))
				position -= forward * speed * dt;
			if (Input::IsKeyPressed(key::A))
				position -= right * speed * dt;
			if (Input::IsKeyPressed(key::D))
				position += right * speed * dt;
			if (Input::IsKeyPressed(key::E))
				position.y += speed * dt;
			if (Input::IsKeyPressed(key::Q))
				position.y -= speed * dt;

			_position = position;

			double xpos, ypos;
			xpos = -Input::GetMousePosition().x;
			ypos = Input::GetMousePosition().y;

			if (_firstMouse) {
				_lastX = xpos;
				_lastY = ypos;
				_firstMouse = false;
			}

			float xoffset = static_cast<float>(xpos - _lastX);
			float yoffset = static_cast<float>(_lastY - ypos);

			_lastX = xpos;
			_lastY = ypos;

			xoffset *= 1.0f * dt;
			yoffset *= 1.0f * dt;

			Orbit(yoffset, xoffset);
		}

		CalculateCameraMatrixes();
	}
}