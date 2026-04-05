#pragma once
#include <chrono>
#include <string>
#include <iostream>
#include <fstream>
#include "Logger.h"

namespace ge::profiler {
	struct ProfileEvent {
		std::string Name;
		double StartMS;
		double DurationMS;
	};
	class Profiler {
	public:
		static void Init() {
			_instance = new Profiler();
		}
		static void Destroy() {
			delete _instance;
			_instance = nullptr;
		}

		static Profiler& Get() {
			return *_instance;
		}

		void AddEvent(const ProfileEvent& event) {
			std::lock_guard<std::mutex> lock(_mutex);
			_events.push_back(event);
		}
		const std::vector<ProfileEvent>& GetEvents() const {
			return _events;
		}
		void Clear() {
			_events.clear();
		}
	private:
		inline static Profiler* _instance = nullptr;
		std::vector<ProfileEvent> _events;
		std::mutex _mutex;
	};
	namespace utils {
		inline double GetTimeMS() {
			static auto start = std::chrono::high_resolution_clock::now();
			auto now = std::chrono::high_resolution_clock::now();
			return std::chrono::duration<double, std::milli>(now - start).count();
		}

		static void WriteProfileToFile(const std::string& path) {
			std::ofstream file(path);
			file << "{ \"traceEvents\": [";
			const auto& events = Profiler::Get().GetEvents();
			for (size_t i = 0; i < events.size(); i++) {
				const auto& e = events[i];
				file << "{";
				file << "\"name\":\"" << e.Name << "\",";
				file << "\"ph\":\"X\",";
				file << "\"ts\":" << e.StartMS << ",";
				file << "\"dur\":" << e.DurationMS << ",";
				file << "\"pid\":0,\"tid\":0";
				file << "}";

				if (i + 1 < events.size())
					file << ",";
			}
			file << "] }";
		}
	}

	class ProfileScope {
	public:
		ProfileScope(const std::string& name) : _active(true), _name(name), _start(utils::GetTimeMS()) {}

		void Stop() {
			if (!_active) return;
			_active = false;

			double end = utils::GetTimeMS();
			Profiler::Get().AddEvent({ _name, _start, end - _start });
		}

		~ProfileScope() {
			Stop();
		}
	private:
		std::string _name;
		bool _active;
		double _start;
	};
}