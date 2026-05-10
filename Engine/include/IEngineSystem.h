#pragma once
#include <Core.h>
#include <Memory/Memory.h>

namespace ge {
	class IEngineSystem : public mem::RefCounted {
	public:
		virtual ~IEngineSystem() = default;
		virtual void OnCreate() = 0;
		virtual void OnDestroy() = 0;
		virtual void OnUpdate(float deltaTime) {}
		virtual const char* GetDebugName() = 0;
	};
}