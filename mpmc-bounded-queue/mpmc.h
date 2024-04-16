#pragma once

#include <atomic>
#include <cassert>
#include <thread>
#include <vector>

// static const size_t kMaxSize = 1'500'000;

template <class T>
class MPMCBoundedQueue {
private:
    struct alignas(64) Element {
        T data;
        std::atomic<uint64_t> epoch;
    };

public:
    explicit MPMCBoundedQueue(size_t size) : max_size_{size}, elements_(max_size_) {
        // assert((max_size_ > 0) && ((max_size_ & (max_size_ - 1)) == 0));
        // assert(max_size_ <= kMaxSize);
        for (uint64_t index = 0; index < max_size_; ++index) {
            elements_[index].epoch.store(index, std::memory_order_relaxed);
        }
    }

    bool Enqueue(const T& value) {
        uint64_t prev_tail = tail_.load(std::memory_order_relaxed);
        Element* element = nullptr;
        while (true) {
            element = &elements_[prev_tail & (max_size_ - 1)];
            if (prev_tail > element->epoch.load(std::memory_order_acquire)) {
                return false;
            }
            if (tail_.compare_exchange_weak(prev_tail, prev_tail + 1, std::memory_order_acquire,
                                            std::memory_order_relaxed)) {
                break;
            }
            std::this_thread::yield();
        }
        element->data = value;
        element->epoch.fetch_add(1, std::memory_order_release);
        return true;
    }

    bool Dequeue(T& data) {
        uint64_t prev_head = head_.load(std::memory_order_relaxed);
        Element* element = nullptr;
        while (true) {
            element = &elements_[prev_head & (max_size_ - 1)];
            if (prev_head + 1 > element->epoch.load(std::memory_order_acquire)) {
                return false;
            }
            if (head_.compare_exchange_weak(prev_head, prev_head + 1, std::memory_order_acquire,
                                            std::memory_order_relaxed)) {
                break;
            }
            std::this_thread::yield();
        }
        data = std::move(element->data);
        element->epoch.fetch_add(max_size_ - 1, std::memory_order_release);
        return true;
    }

private:
    const size_t max_size_;
    // alignas(64) std::atomic<int64_t> size_{0};
    alignas(64) std::atomic<uint64_t> head_{0};
    alignas(64) std::atomic<uint64_t> tail_{0};
    // std::array<Element, kMaxSize> elements_;
    alignas(64) std::vector<Element> elements_;
};
