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
    //singleton pattern
    static TimingLogger& instance();

    Timer& newTimer(std::string timerName);

    void logTimers();

private:
    TimingLogger() = default;
    TimingLogger(const TimingLogger&) = delete;
    TimingLogger(TimingLogger&&) = delete;
    TimingLogger& operator=(const TimingLogger&) = delete;
    TimingLogger& operator=(TimingLogger&&) = delete;
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