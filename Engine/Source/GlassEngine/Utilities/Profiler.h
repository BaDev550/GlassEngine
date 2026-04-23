#pragma once

#include <chrono>
#include <string>
#include <fstream>
#include <filesystem>

namespace ge::profile {
	struct ProfileEvent {
		std::string name;
		double startMS;
		double durationMS;
	};

	class Profiler {
	public:
		static void Init() {
			if (_instance) {
				std::cout << "Profiler already initialized" << std::endl;
				return;
			}
			_instance = new Profiler();
		}
		static void Destroy() {
			delete _instance;
			_instance = nullptr;
		}

		static Profiler& Get() { return *_instance; }
		void AddEvent(const std::string& name, double startMS, double durationMS) {
			_events.push_back({ name, startMS, durationMS });
		}
		const std::vector<ProfileEvent>& GetEvents() const { return _events; }
		void Clear() { _events.clear(); }
		bool IsValid() const { return _instance != nullptr; }
	private:
		inline static Profiler* _instance = nullptr;
		std::vector<ProfileEvent> _events;
	};

	namespace utils {
		inline double GetCurrentTimeMS() {
			static auto startTime = std::chrono::high_resolution_clock::now();
			auto now = std::chrono::high_resolution_clock::now();
			return std::chrono::duration<double, std::milli>(now - startTime).count();
		}

		static void WriteEventsToFile(const std::vector<ProfileEvent>& events, std::filesystem::path filePath) {
			filePath.replace_extension(".json");
			std::ofstream out(filePath);
			if (!out.is_open()) {
				std::cout << "Failed to open profile output file: " << filePath.string() << std::endl;
				return;
			}
			out << "{\"traceEvents\":[";
			for (size_t i = 0; i < events.size(); i++) {
				const auto& event = events[i];
				out << "{";
				out << "\"name\":\"" << event.name << "\",";
				out << "\"ph\":\"X\",";
				out << "\"ts\": " << event.startMS << ",";
				out << "\"dur\": " << event.durationMS << ",";
				out << "\"pid\": 0,";
				out << "\"tid\": " << std::this_thread::get_id();
				out << "}";

				if (i != events.size() - 1)
					out << ",";
			}
			out << "]}";
			out.close();
			std::cout << "Profile events written to " << filePath.string() << std::endl;
		}
	}

	class ProfileScope {
	public:
		ProfileScope(const std::string& name) : _name(name), _startMS(utils::GetCurrentTimeMS()) {}
		~ProfileScope() {
			Stop();
		}

		void Stop() {
			if (!Profiler::Get().IsValid()) return;

			if (!_stopped) {
				double endMS = utils::GetCurrentTimeMS();
				double duration = endMS - _startMS;
				Profiler::Get().AddEvent(_name, _startMS, duration);
				_stopped = true;
			}
		}
	private:
		std::string _name;
		double _startMS;
		bool _stopped = false;
	};

#define GE_PROFILE_SCOPE(name) ge::profile::ProfileScope profileScope##__LINE__(name)
}