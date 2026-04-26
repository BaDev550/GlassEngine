#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GlassEngine/Core/Core.h"

namespace ge::renderer {
	class Camera : public ge::mem::RefCounted {
	public:
		virtual ~Camera() = default;
		virtual void Update(float dt) {}

		void SetFarPlane(float farplane) { _farPlane = farplane; }
		void SetNearPlane(float nearplane) { _nearPlane = nearplane; }
		void Orbit(float pitchOffset, float yawOffset);
		void SetFirstMouse() { _firstMouse = true; }
		void SetPosition(const glm::vec3& pos) { _position = pos; }
		void SetRotation(const glm::vec3& rot) { _direction = rot; }
		void SetForward(const glm::vec3& forward) { _forward = forward; }
		float GetFarPlane() const { return _farPlane; }
		float GetNearPlane() const { return _nearPlane; }
		glm::mat4 GetView() const { return _viewMatrix; }
		glm::mat4 GetViewProjection() const { return _viewMatrix * _projectionMatrix; }
		glm::mat4 GetProjection() const { return _projectionMatrix; }
		glm::vec3 GetPosition() const { return _position; }
		glm::vec3 GetForward() const { return _forward; }
		glm::vec3 GetRight() const { return _right; }
		void CalculateCameraMatrixes();
	protected:
		void AddPitch(float value);
		void AddYaw(float value);

		float _farPlane = 1000.0f;
		float _nearPlane = 0.1f;
		bool _firstMouse = false;
		glm::mat4 _viewMatrix = glm::mat4(1.0f);
		glm::mat4 _projectionMatrix = glm::mat4(1.0f);
		glm::vec3 _position = glm::vec3(0.0f, 0.0f, 3.0f);
		glm::vec3 _direction = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 _up = glm::vec3(0.0, -1.0f, 0.0f);
		glm::vec3 _forward = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 _right = glm::vec3(0.0f, 0.0f, 0.0f);
	};
}