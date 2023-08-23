#ifndef TIMING
#define TIMING

#include <string>
#include <chrono>
#include <map>

class Timer;

class TimingLogger {
private:
    std::map<std::string, Timer> timings;

public:
    static TimingLogger& instance();

    Timer& newTimer(std::string timerName);

    void logTimers();

private:
    TimingLogger() = default;
};

class Timer {
private:
    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::chrono::time_point<std::chrono::steady_clock> end_time;

public:
    Timer();
    ~Timer();

    void stopTimer();
    std::chrono::nanoseconds getElapsedTime() const;    
};

#endif