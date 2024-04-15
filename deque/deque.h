#pragma once
#include <iterator>
#include <iostream>
#include <cstddef>
#include <initializer_list>
#include <algorithm>

class Deque {
public:
    Deque() {
        Initialize();
    }
    Deque(const Deque& other) {
        num_blocks_ = other.num_blocks_;
        blocks_ = new int*[num_blocks_];
        head_ind_ = other.head_ind_;
        tail_ind_ = other.tail_ind_;
        size_ = other.size_;
        for (size_t ind = 0; ind < num_blocks_; ++ind) {
            if (other.blocks_[ind]) {
                blocks_[ind] = new int[kBlockSize];
                std::copy(other.blocks_[ind], other.blocks_[ind] + kBlockSize, blocks_[ind]);
            } else {
                blocks_[ind] = nullptr;
            }
        }
        auto head_block_dist = std::distance(other.blocks_, other.head_block_ptr_);
        head_block_ptr_ = std::next(blocks_, head_block_dist);
        auto tail_block_dist = std::distance(other.blocks_, other.tail_block_ptr_);
        tail_block_ptr_ = std::next(blocks_, tail_block_dist);
    }
    Deque(Deque&& other) {
        Swap(other);
        other.Initialize();
    }

    explicit Deque(size_t size) : Deque() {
        size_t new_num_blocks = num_blocks_;
        while (kBlockSize * new_num_blocks < size) {
            new_num_blocks *= kReallocationCoef;
        }
        Reallocate(new_num_blocks);

        //  for (int ind = (static_cast<int>(size) / 2) - 1; ind >= 0; --ind) {
        //     PushFront(0);
        // }
        for (size_t ind = 0; ind < size; ++ind) {
            PushBack(0);
        }
    }

    Deque(std::initializer_list<int> list) : Deque() {
        size_t new_num_blocks = num_blocks_;
        while (kBlockSize * new_num_blocks < list.size()) {
            new_num_blocks *= kReallocationCoef;
        }
        Reallocate(new_num_blocks);

        for (auto it = list.begin() + (list.size() / 2) - 1; it >= list.begin(); --it) {
            PushFront(*it);
        }
        for (auto it = list.begin() + (list.size() / 2); it < list.end(); ++it) {
            PushBack(*it);
        }
    }
    ~Deque() {
        for (size_t ind = 0; ind < num_blocks_; ++ind) {
            if (blocks_[ind]) {
                delete[] blocks_[ind];
            }
        }
        delete[] blocks_;
    }

    Deque& operator=(const Deque& other) {
        if (&other != this) {
            Deque tmp(other);
            Swap(tmp);
        }
        return *this;
    }
    Deque& operator=(Deque&& other) {
        this->~Deque();
        Swap(other);
        other.Initialize();
        return *this;
    }

    void Swap(Deque& other) {
        std::swap(blocks_, other.blocks_);
        std::swap(num_blocks_, other.num_blocks_);
        std::swap(head_block_ptr_, other.head_block_ptr_);
        std::swap(tail_block_ptr_, other.tail_block_ptr_);
        std::swap(head_ind_, other.head_ind_);
        std::swap(tail_ind_, other.tail_ind_);
        std::swap(size_, other.size_);
    }

    void PushBack(int value) {
        if (tail_ind_ >= static_cast<int>(kBlockSize)) {
            if (tail_block_ptr_ >= blocks_ + num_blocks_) {
                Reallocate(kReallocationCoef * num_blocks_);
            }
            if (!*tail_block_ptr_) {
                *tail_block_ptr_ = new int[kBlockSize];
            }
            ++tail_block_ptr_;
            tail_ind_ = 0;
        }
        (*(tail_block_ptr_ - 1))[tail_ind_] = value;
        ++tail_ind_;
        ++size_;
    }

    void PopBack() {
        if (!size_) {
            return;
        }
        --tail_ind_;
        --size_;
        if (tail_ind_ < 0) {
            tail_ind_ = kBlockSize - 1;
            if (*tail_block_ptr_) {
                delete[] *tail_block_ptr_;
                *tail_block_ptr_ = nullptr;
            }
            --tail_block_ptr_;
        }
    }

    void PushFront(int value) {
        if (head_ind_ <= -1) {
            if (head_block_ptr_ < blocks_) {
                Reallocate(kReallocationCoef * num_blocks_);
            }
            if (!*head_block_ptr_) {
                *head_block_ptr_ = new int[kBlockSize];
            }
            --head_block_ptr_;
            head_ind_ = kBlockSize - 1;
        }
        (*(head_block_ptr_ + 1))[head_ind_] = value;
        --head_ind_;
        ++size_;
    }

    void PopFront() {
        if (!size_) {
            return;
        }
        ++head_ind_;
        --size_;
        if (head_ind_ >= static_cast<int>(kBlockSize)) {
            head_ind_ = 0;
            if (*head_block_ptr_) {
                delete[] *head_block_ptr_;
                *head_block_ptr_ = nullptr;
            }
            ++head_block_ptr_;
        }
    }

    int& operator[](size_t index) {
        int** block_ptr;
        size_t first_block_num_elems = kBlockSize - head_ind_ - 1;
        if (first_block_num_elems > index) {
            block_ptr = head_block_ptr_ + 1;
            index = head_ind_ + index + 1;
        } else {
            index -= first_block_num_elems;
            block_ptr = head_block_ptr_ + 2 + (index / kBlockSize);
            index %= kBlockSize;
        }
        return (*block_ptr)[index];
    }

    const int& operator[](size_t index) const {
        int** block_ptr;
        size_t first_block_num_elems = kBlockSize - head_ind_ - 1;
        if (first_block_num_elems > index) {
            block_ptr = head_block_ptr_ + 1;
            index = head_ind_ + index + 1;
        } else {
            index -= first_block_num_elems;
            block_ptr = head_block_ptr_ + 2 + (index / kBlockSize);
            index %= kBlockSize;
        }
        return (*block_ptr)[index];
    }

    size_t Size() const {
        return size_;
    }

    void Clear() {
        this->~Deque();
        Initialize();
    }

private:
    const static size_t kReallocationCoef = 2;
    const static size_t kBlockSize = 128;
    int** blocks_;
    size_t num_blocks_;
    int** head_block_ptr_;
    int** tail_block_ptr_;
    int head_ind_;
    int tail_ind_;
    size_t size_;

    void Initialize() {
        num_blocks_ = 2;
        blocks_ = new int*[num_blocks_];
        std::fill_n(blocks_, num_blocks_, nullptr);
        head_block_ptr_ = (blocks_ + (num_blocks_ / 2) - 1);
        tail_block_ptr_ = (blocks_ + (num_blocks_ / 2));
        head_ind_ = -1;
        tail_ind_ = kBlockSize;
        size_ = 0;
    }

    void Reallocate(size_t new_num_blocks) {
        if (new_num_blocks <= num_blocks_) {
            return;
        }
        int** new_blocks = new int*[new_num_blocks];
        int** old_blocks_begin = new_blocks + ((new_num_blocks - num_blocks_) / 2);
        int** old_blocks_end = old_blocks_begin + num_blocks_;
        std::fill(new_blocks, old_blocks_begin, nullptr);
        std::fill(old_blocks_end, new_blocks + new_num_blocks, nullptr);
        std::copy(blocks_, blocks_ + num_blocks_, old_blocks_begin);
        head_block_ptr_ = old_blocks_begin + (head_block_ptr_ - blocks_);
        tail_block_ptr_ = old_blocks_begin + (tail_block_ptr_ - blocks_);
        delete[] blocks_;
        blocks_ = new_blocks;
        num_blocks_ = new_num_blocks;
    }
};

std::ostream& operator<<(std::ostream& out, const Deque& deq) {
    for (size_t ind = 0; ind < deq.Size(); ++ind) {
        out << deq[ind] << ' ';
    }
    return out;
}
