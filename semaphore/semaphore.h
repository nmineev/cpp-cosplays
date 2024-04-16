#pragma once

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    explicit Semaphore(int count) : count_{count} {
    }

    void Acquire(auto callback) {
        std::unique_lock lock{mutex_};
        auto thread_ticket = ticket_++;
        cv_.wait(lock, [this, thread_ticket] {
            if (count_ && (thread_ticket <= top_)) {
                return true;
            } else if (count_) {
                cv_.notify_one();
            }
            return false;
        });
        callback(count_);
        ++top_;
    }

    void Acquire() {
        Acquire([](int& value) { --value; });
    }

    void Release() {
        std::lock_guard lock{mutex_};
        ++count_;
        cv_.notify_one();
    }

private:
    int count_;
    int64_t ticket_{0};
    int64_t top_{0};
    std::mutex mutex_;
    std::condition_variable cv_;
};
