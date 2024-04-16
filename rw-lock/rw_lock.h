#pragma once

#include <atomic>

class RWLock {
public:
    void Read(auto func) {
        atm_.fetch_add(1);
        uint32_t old = atm_.load();
        while (GetWriters(old) > 0) {
            atm_.wait(old);
            old = atm_.load();
        }
        try {
            func();
        } catch (...) {
            EndRead();
            throw;
        }
        EndRead();
    }

    void Write(auto func) {
        uint32_t zero = 0;
        while (!atm_.compare_exchange_strong(zero, kOneWriter)) {
            atm_.wait(zero);
            zero = 0;
        }
        func();
        atm_.fetch_and((static_cast<uint32_t>(1) << 16) - 1);
        atm_.notify_all();
    }

private:
    void EndRead() {
        if (atm_.fetch_sub(1) == 1) {
            atm_.notify_all();
        }
    }

    uint32_t GetWriters(uint32_t value) {
        return value >> 16;
    }

    uint32_t GetReaders(uint32_t value) {
        return (value & ((static_cast<uint32_t>(1) << 16) - 1));
    }

private:
    static const uint32_t kOneWriter = static_cast<uint32_t>(1) << 16;

    std::atomic<uint32_t> atm_{0};
};
