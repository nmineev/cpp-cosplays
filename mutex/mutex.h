#pragma once

#include <atomic>

#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/time.h>

// Atomically do the following:
//    if (*value == expected_value) {
//        sleep_on_address(value)
//    }
inline void FutexWait(int* value, int expected_value) {
    syscall(SYS_futex, value, FUTEX_WAIT_PRIVATE, expected_value, nullptr, nullptr, 0);
}

// Wakeup 'count' threads sleeping on address of value (-1 wakes all)
inline void FutexWake(int* value, int count) {
    syscall(SYS_futex, value, FUTEX_WAKE_PRIVATE, count, nullptr, nullptr, 0);
}

class Mutex {
public:
    void Lock() {
        if (!TryLock()) {
            Parking();
        }
    }

    void Unlock() {
        auto locked_atm = std::atomic_ref<int>(locked_);
        if (locked_atm.fetch_sub(1, std::memory_order::release) != 1) {
            locked_atm.store(0, std::memory_order::release);
            FutexWake(&locked_, 1);
        }
    }

    // BasicLockable
    // https://en.cppreference.com/w/cpp/named_req/BasicLockable

    void lock() {
        Lock();
    }

    void unlock() {
        Unlock();
    }

private:
    int locked_{0};

    bool TryLock() {
        const size_t yield_limit = 0;
        size_t yield_count = 1;
        int zero = 0;
        auto locked_atm = std::atomic_ref<int>(locked_);
        while (!locked_atm.compare_exchange_weak(zero, 1, std::memory_order::acquire,
                                                 std::memory_order::relaxed)) {
            zero = 0;
            if (yield_count > yield_limit) {
                return false;
            }
            ++yield_count;
        }
        return true;
    }

    void Parking() {
        auto locked_atm = std::atomic_ref<int>(locked_);
        do {
            if (locked_atm.exchange(2, std::memory_order::acquire) == 0) {
                return;
            }
            FutexWait(&locked_, /*old=*/2);
        } while (!TryLock());
        locked_atm.store(2, std::memory_order::relaxed);
    }
};
