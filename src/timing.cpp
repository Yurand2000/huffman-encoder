#include "timing.h"

#include <iostream>

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

std::chrono::milliseconds Timer::getElapsedTime() const {
    if (end_time != std::chrono::time_point<std::chrono::steady_clock>()) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    } else {
        return std::chrono::milliseconds(0);
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
    for(auto& pair : timings) {
        std::cout << pair.first << " - " << pair.second.getElapsedTime().count() << "ms" << std::endl;
    }
}