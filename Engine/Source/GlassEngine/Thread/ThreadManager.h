#pragma once
#include <mutex>
#include <thread>
#include <functional>
#include <future>
#include <queue>
#include <memory>
#include <condition_variable>
#include "GlassEngine/Core/Core.h"

namespace ge {
	class ThreadManager {
	public:
		explicit ThreadManager(size_t numThreads);
		~ThreadManager();

		template<class F, class... Args>
		auto Enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>; // future to return functions value
	private:
		GEVector<std::thread> _workers;
		std::queue<std::function<void()>> _tasks;
		std::mutex _queueMutex;
		std::condition_variable _condition;
		bool _stopThread = false;
	};

	template<class F, class ...Args>
	auto ThreadManager::Enqueue(F&& f, Args && ...args) -> std::future<typename std::invoke_result<F, Args ...>::type>
	{
		using return_type = typename std::invoke_result<F, Args...>::type;
		auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
		std::future<return_type> res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(_queueMutex);
			GE_ASSERT(!_stopThread, "Enqueue called on stoped thread manager");
			_tasks.emplace([task]() {(*task)(); });
		}
		_condition.notify_one();
		return res;
	}
}