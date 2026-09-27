#pragma once

#include <thread>
#include <atomic>
#include <functional>
#include <chrono>
#include <mutex>

class ElectionTimer {
public:
    // onTimeout is called when the timer fires without being reset in time.
    ElectionTimer(int minMs, int maxMs, std::function<void()> onTimeout);
    ~ElectionTimer();

    void start();
    void reset();   // call this whenever a valid heartbeat/vote is received
    void stop();

private:
    void run();
    int randomTimeoutMs();

    int minMs_;
    int maxMs_;
    std::function<void()> onTimeout_;

    std::thread thread_;
    std::atomic<bool> running_;
    std::atomic<bool> resetRequested_;
    std::chrono::steady_clock::time_point deadline_;
    std::mutex deadlineMutex_;
};