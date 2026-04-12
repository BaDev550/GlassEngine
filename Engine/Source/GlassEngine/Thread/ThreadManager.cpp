#include "gepch.h"
#include "ThreadManager.h"

namespace ge {
	ThreadManager::ThreadManager(size_t numThreads)
	{
		for (size_t i = 0; i < numThreads; i++) {
			_workers.emplace_back([this] {
				while (true) {
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(this->_queueMutex);
						this->_condition.wait(lock, [this] {
							return this->_stopThread || !this->_tasks.empty();
							});
						if (this->_stopThread && this->_tasks.empty())
							return;

						task = std::move(this->_tasks.front());
						this->_tasks.pop();
					}
					task();
				}
				});
		}
	}

	ThreadManager::~ThreadManager()
	{
		{
			std::unique_lock<std::mutex> lock(_queueMutex);
			_stopThread = true;
		}
		_condition.notify_all();

		for (std::thread& worker : _workers) {
			if (worker.joinable())
				worker.join();
		}
	}
}