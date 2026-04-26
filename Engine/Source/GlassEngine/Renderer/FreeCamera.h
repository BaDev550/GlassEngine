#pragma once
#include "Camera.h"

namespace ge::renderer {
	class FreeCamera : public Camera {
	public:
		FreeCamera() = default;

		virtual void Update(float dt) override;
		void SetProccessingMouse(bool processing) { _proccessingMouse = processing; }
	private:
		float _lastX = 0.0f;
		float _lastY = 0.0f;
		bool _proccessingMouse = true;
	};
}