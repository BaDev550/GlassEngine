#pragma once

#include <chrono>
#include <string>
#include <fstream>
#include <filesystem>
#include "GlassEngine/Core/Core.h"

namespace ge::profile {
	struct ProfileEvent {
		GEString name;
		double startMS;
		double durationMS;
	};

	class Profiler {
	public:
		static void Init() {
			if (_instance) {
				GE_CORE_ERROR("Profiler already initialized");
				return;
			}
			_instance = new Profiler();
		}
		static void Destroy() {
			delete _instance;
			_instance = nullptr;
		}

		static Profiler& Get() { return *_instance; }
		void AddEvent(const GEString& name, double startMS, double durationMS) {
			_events.push_back({ name, startMS, durationMS });
		}
		const GEVector<ProfileEvent>& GetEvents() const { return _events; }
		void Clear() { _events.clear(); }
	private:
		inline static Profiler* _instance = nullptr;
		GEVector<ProfileEvent> _events;
	};

	namespace utils {
		inline double GetCurrentTimeMS() {
			static auto startTime = std::chrono::high_resolution_clock::now();
			auto now = std::chrono::high_resolution_clock::now();
			return std::chrono::duration<double, std::milli>(now - startTime).count();
		}

		static void WriteEventsToFile(const GEVector<ProfileEvent>& events, std::filesystem::path filePath) {
			filePath.replace_extension(".json");
			std::ofstream out(filePath);
			if (!out.is_open()) {
				GE_CORE_ERROR("Failed to open profile output file: {}", filePath.string());
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
			GE_CORE_INFO("Profile events written to {}", filePath.string());
		}
	}

	class ProfileScope {
	public:
		ProfileScope(const GEString& name) : _name(name), _startMS(utils::GetCurrentTimeMS()) {}
		~ProfileScope() {
			Stop();
		}

		void Stop() {
			if (!_stopped) {
				double endMS = utils::GetCurrentTimeMS();
				double duration = endMS - _startMS;
				Profiler::Get().AddEvent(_name, _startMS, duration);
				_stopped = true;
			}
		}
	private:
		GEString _name;
		double _startMS;
		bool _stopped = false;
	};

#define GE_PROFILE_SCOPE(name) ge::profile::ProfileScope profileScope##__LINE__(name)
}