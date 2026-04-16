#pragma once
#include <string>
#include <chrono>

namespace ge {
	class Time {
	public:
		static std::string GetCurrentLocalTime() {
			auto now = std::chrono::system_clock::now();
			auto localTime = std::chrono::current_zone()->to_local(now);
			auto localTimeInSeconds = std::chrono::floor<std::chrono::seconds>(localTime);
			return std::format("{:%Y-%m-%d %H:%M:%S}", localTimeInSeconds);
		}
	};
}