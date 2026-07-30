#include "timer.h"
#include <iomanip>

std::chrono::time_point<std::chrono::high_resolution_clock> Timer::start_t = std::chrono::high_resolution_clock::now();
std::chrono::time_point<std::chrono::high_resolution_clock> Timer::start_t_stopwatch = std::chrono::high_resolution_clock::now();
std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> Timer::times;
std::map<size_t, std::chrono::time_point<std::chrono::high_resolution_clock>> Timer::alarms;

void Timer::start() {
    start_t = std::chrono::high_resolution_clock::now();
}

void Timer::start_alarm(const size_t id) {
    // Do it twice so that if it needs to be constructed the first time, we don't account for the map construction time.
    // Only inaccuracy is map access time here.
    alarms[id] = std::chrono::high_resolution_clock::now();
    alarms[id] = std::chrono::high_resolution_clock::now();
}

void Timer::end_alarm(const size_t id, int precision, std::string msg) {
    auto end_t = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_t - alarms[id];
    double time_ms = 1000.0 * elapsed.count();
    if (precision <= 0) precision = 2;
    std::cout << msg << std::fixed << std::setprecision(precision) << "(" << time_ms << "ms)" << std::endl;
}

void Timer::start_stopwatch(size_t expected_trials) {
    times.clear();
    if (expected_trials) times.reserve(expected_trials);
    start_t_stopwatch = std::chrono::high_resolution_clock::now();
}

void Timer::lap_stopwatch() {
    // Total trials could be tracked by incrementing a counter, but use a vec here for min/max testing if
    // We need it later
    times.emplace_back(std::chrono::high_resolution_clock::now());
}

void Timer::end_stopwatch(int precision, std::string msg) {
    auto end_t = std::chrono::high_resolution_clock::now();
    times.emplace_back(end_t);
    std::chrono::duration<double> total_elapsed = end_t - start_t_stopwatch;
    double total_time_ms = 1000.0 * total_elapsed.count();
    double average_ms = total_time_ms / (double)times.size();
    if (precision <= 0) precision = 2;
    std::cout << msg << std::fixed << std::setprecision(precision) << "(" << total_time_ms << "ms total\t|\t" << average_ms << "ms average)" << std::endl;
}

void Timer::end(int precision, std::string msg) {
    auto end_t = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_t - start_t;
    double time_ms = 1000.0 * elapsed.count();
    if (precision <= 0) precision = 2;
    std::cout << msg << std::fixed << std::setprecision(precision) << "(" << time_ms << "ms)" << std::endl;
}