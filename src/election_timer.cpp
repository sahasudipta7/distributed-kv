#include "election_timer.h"
#include <random>

ElectionTimer::ElectionTimer(int minMs, int maxMs, std::function<void()> onTimeout)
    : minMs_(minMs), maxMs_(maxMs), onTimeout_(onTimeout), running_(false), resetRequested_(false) {
}

ElectionTimer::~ElectionTimer() {
    stop();
}

int ElectionTimer::randomTimeoutMs() {
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(minMs_, maxMs_);
    return dist(rng);
}

void ElectionTimer::start() {
    running_ = true;
    {
        std::lock_guard<std::mutex> lock(deadlineMutex_);
        deadline_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(randomTimeoutMs());
    }
    thread_ = std::thread(&ElectionTimer::run, this);
}

void ElectionTimer::reset() {
    std::lock_guard<std::mutex> lock(deadlineMutex_);
    deadline_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(randomTimeoutMs());
}

void ElectionTimer::stop() {
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
}

void ElectionTimer::run() {
    while (running_) {
        std::chrono::steady_clock::time_point currentDeadline;
        {
            std::lock_guard<std::mutex> lock(deadlineMutex_);
            currentDeadline = deadline_;
        }

        if (std::chrono::steady_clock::now() >= currentDeadline) {
            onTimeout_();
            // After firing, give ourselves a new deadline so we don't
            // fire repeatedly in a tight loop before something resets us.
            std::lock_guard<std::mutex> lock(deadlineMutex_);
            deadline_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(randomTimeoutMs());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}