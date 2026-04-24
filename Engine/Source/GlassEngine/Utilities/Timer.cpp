#include "gepch.h"
#include "Timer.h"

namespace ge {
    Timer::Timer() {
        Reset();
    }

    void Timer::Reset() {
        _start = std::chrono::high_resolution_clock::now();
    }

    void Timer::Update(float deltaTime) {}

    float Timer::Elapsed() const { return std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - _start).count(); }
    float Timer::ElapsedMillis() const { return Elapsed() * 1000.0f; }
}