#pragma once

namespace ge::renderer {
	class CommandBuffer {
	public:
		virtual ~CommandBuffer() = default;
		virtual void Begin() = 0;
		virtual void End() = 0;
	};
}