#include "timing.h"

#include <iostream>
#include <iomanip>

Timer::Timer() {
    start_time = std::chrono::steady_clock::now();
}

Timer::~Timer() {
    stopTimer();
}

void Timer::stopTimer() {
    if (end_time == std::chrono::time_point<std::chrono::steady_clock>()) {
        end_time = std::chrono::steady_clock::now();
    }
}

std::chrono::nanoseconds Timer::getElapsedTime() const {
    if (end_time != std::chrono::time_point<std::chrono::steady_clock>()) {
        return end_time - start_time;
    } else {
        auto now = std::chrono::steady_clock::now();
        return now - start_time;
    }
}

TimingLogger& TimingLogger::instance() {
    static TimingLogger logger;
    return logger;
}

Timer& TimingLogger::newTimer(std::string timerName) {
    auto pair = timings.emplace(timerName, Timer());
    return (*pair.first).second;
}

void TimingLogger::logTimers() {
    std::cout << std::setw(12) << std::right << "Time (ms)" << " | " << std::left << "Timer/Sequence Name" << std::endl;
    for(auto const& [timer_name, timer] : timings) {
        auto elapsed_time = timer.getElapsedTime();
        auto elapsed_time_ms = elapsed_time.count() / 1000000.0;
        std::cout << std::setw(12) << std::right << std::fixed << 
            std::setprecision(3) << elapsed_time_ms << " | " <<
            std::left << timer_name << std::endl;
    }
}