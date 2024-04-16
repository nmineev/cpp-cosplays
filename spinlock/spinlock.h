#pragma once

#include <atomic>
#include <thread>

class SpinLock {
public:
    void Lock() {
        size_t max_num_tries = 10;
        size_t num_tries = 0;
        while (locked_.exchange(true, std::memory_order_acquire)) {
            while (locked_.load(std::memory_order_relaxed)) {
                if (++num_tries >= max_num_tries) {
                    std::this_thread::yield();
                    num_tries = 0;
                }
            }
        }
    }

    void Unlock() {
        locked_.store(false, std::memory_order_release);
    }

private:
    std::atomic<bool> locked_{false};
};
