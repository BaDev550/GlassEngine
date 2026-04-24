#pragma once
#include <string>
#include <chrono>

namespace ge {
    class TimeStep {
    public:
        TimeStep(float time = 0.0f) : _time(time) {}
        operator float() const { return _time; }

        float GetSeconds() const { return _time; }
        float GetMilliseconds() const { return _time * 1000.0f; }
    private:
        float _time;
    };

    class Timer {
	public:
        Timer();
        void Reset();
        void Update(float deltaTime);
        float Elapsed() const;
        float ElapsedMillis() const;

        static std::string GetCurrentLocalTime() {
            auto now = std::chrono::system_clock::now();
            auto localTime = std::chrono::current_zone()->to_local(now);
            auto localTimeInSeconds = std::chrono::floor<std::chrono::seconds>(localTime);
            return std::format("{:%Y-%m-%d %H.%M.%S}", localTimeInSeconds);
        }
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> _start;
    };
}