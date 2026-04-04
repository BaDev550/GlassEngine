#pragma once

namespace ge {
	class Layer {
	public:
		Layer(const char* debugName = "Layer") : _debugName(debugName) {}
		virtual ~Layer() = default;
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnEvent() {}
		inline const char* GetDebugName() const { return _debugName; }
	private:
		const char* _debugName = "Layer";
	};
}