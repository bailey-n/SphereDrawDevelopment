#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <iostream>
#include <string>
#include <map>

class Timer {
    static std::chrono::time_point<std::chrono::high_resolution_clock> start_t;
    static std::chrono::time_point<std::chrono::high_resolution_clock> start_t_stopwatch;
    static std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> times;
    static std::map<size_t, std::chrono::time_point<std::chrono::high_resolution_clock>> alarms;
public:
    static void start();
    static void start_alarm(size_t id);
    static void end_alarm(size_t id, int precision, std::string msg);
    static void start_stopwatch(size_t expected_trials = 0);
    static void lap_stopwatch();
    static void end_stopwatch(int precision, std::string msg);
    static void end(int precision, std::string msg);
};


#endif //TIMER_H
