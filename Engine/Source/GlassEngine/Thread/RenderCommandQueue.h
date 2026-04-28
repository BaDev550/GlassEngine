#pragma once
#include <functional>
#include <vector>
#include <mutex>
#include "GlassEngine/Core/Core.h"

namespace ge {
	class RenderCommandQueue
	{
	public:
		void Submit(std::function<void()> command) {
			std::scoped_lock<std::mutex> lock(_mutex);
			_commands.emplace_back(std::move(command));
		}

		void Execute() {
			GEVector<std::function<void()>, mem::ThreadAllocTag> executionQueue;
			{
				std::scoped_lock<std::mutex> lock(_mutex);
				executionQueue.swap(_commands);
				_commands.clear();
			}
			for (auto& command : executionQueue) {
				command();
			}
		}
	private:
		GEVector<std::function<void()>, mem::ThreadAllocTag> _commands;
		std::mutex _mutex;
	};
}