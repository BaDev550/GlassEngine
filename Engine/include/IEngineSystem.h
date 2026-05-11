#pragma once
#include <Core.h>
#include <Memory/Memory.h>

namespace ge {
	class IEngineSystem : public mem::RefCounted {
	public:
		virtual ~IEngineSystem() = default;
		virtual void OnCreate() {};
		virtual void OnDestroy() {};
		virtual void OnTick(float deltaTime) {}
		virtual const char* GetDebugName() { return "Interface_Engine_default_system_name"; };
	};
}