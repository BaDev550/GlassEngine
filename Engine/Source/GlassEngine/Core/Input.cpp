#include "gepch.h"
#include "Input.h"
#include "Application.h"

namespace ge {
	InputManager::InputManager(GLFWwindow* window) : _window(window) {
		_LastMousePos = _MousePos;
	}

	void InputManager::Update()
	{
		std::memcpy(_PrevKeyStates, _KeyStates, sizeof(_KeyStates));
		std::memcpy(_PrevMouseButtonStates, _MouseButtonStates, sizeof(_MouseButtonStates));

		for (int key = GLFW_KEY_SPACE; key < GLFW_KEY_LAST; ++key)
			_KeyStates[key] = glfwGetKey(_window, key) == GLFW_PRESS;

		for (int button = 0; button < GLFW_MOUSE_BUTTON_LAST; ++button)
			_MouseButtonStates[button] = glfwGetMouseButton(_window, button) == GLFW_PRESS;

		_LastMousePos = _MousePos;
		double x, y;
		glfwGetCursorPos(_window, &x, &y);
		_MousePos = { (float)x, (float)y };
		_MouseDelta = _MousePos - _LastMousePos;
	}

	bool InputManager::IsKeyPressed(KeyCode key) { return _KeyStates[key]; }
	bool InputManager::IsKeyJustPressed(KeyCode key) { return _KeyStates[key] && !_PrevKeyStates[key]; }
	bool InputManager::IsKeyJustReleased(KeyCode key) { return !_KeyStates[key] && _PrevKeyStates[key]; }
	bool InputManager::IsMouseButtonPressed(MouseCode button) { return _MouseButtonStates[button]; }
	bool InputManager::IsMouseButtonJustPressed(MouseCode button) { return _MouseButtonStates[button] && !_PrevMouseButtonStates[button]; }
	bool InputManager::IsMouseButtonJustReleased(MouseCode button) { return !_MouseButtonStates[button] && _PrevMouseButtonStates[button]; }

	glm::vec2 InputManager::GetMousePosition() {
		if (!_window)
			return glm::vec2(0.0);
		return _MousePos;
	}

	glm::vec2 InputManager::GetMouseDelta() { return _MouseDelta; }
	float InputManager::GetMouseX() { return _MousePos.x; }
	float InputManager::GetMouseY() { return _MousePos.y; }
}