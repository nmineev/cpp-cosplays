#pragma once

#include <iterator>
#include <cstddef>

class Vector {
public:
    class Iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = int;
        using difference_type = ptrdiff_t;
        using pointer = int*;
        using reference = int&;

        Iterator() : element_ptr_(nullptr) {
        }
        Iterator(int* element_ptr) : element_ptr_(element_ptr) {
        }
        Iterator(const Iterator& other) : element_ptr_(other.element_ptr_) {
        }

        Iterator& operator=(int* element_ptr) {
            element_ptr_ = element_ptr;
            return *this;
        }
        Iterator& operator=(const Iterator& other) {
            element_ptr_ = other.element_ptr_;
            return *this;
        }
        Iterator& operator+=(difference_type n) {
            element_ptr_ += n;
            return *this;
        }
        Iterator& operator-=(difference_type n) {
            element_ptr_ -= n;
            return *this;
        }

        reference operator[](difference_type n) const {
            return *(element_ptr_ + n);
        }
        reference operator*() const {
            return *element_ptr_;
        }
        pointer operator->() {
            return element_ptr_;
        }

        Iterator& operator++() {
            ++element_ptr_;
            return *this;
        }
        Iterator& operator--() {
            --element_ptr_;
            return *this;
        }
        Iterator operator++(int) {
            Iterator copy = element_ptr_;
            ++element_ptr_;
            return copy;
        }
        Iterator operator--(int) {
            Iterator copy = element_ptr_;
            --element_ptr_;
            return copy;
        }
        int* element_ptr_;
    };

    Vector() : elements_(new int[0]), size_(0), capacity_(0) {
    }
    Vector(size_t size) : elements_(new int[size]), size_(size), capacity_(size) {
        for (size_t ind = 0; ind < size; ++ind) {
            elements_[ind] = 0;
        }
    }
    Vector(std::initializer_list<int> other)
        : elements_(new int[other.size()]), size_(other.size()), capacity_(other.size()) {
        std::copy(other.begin(), other.end(), elements_);
    }
    ~Vector() {
        delete[] elements_;
        elements_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    Vector(const Vector& other)
        : elements_(new int[other.size_]), size_(other.size_), capacity_(other.size_) {
        std::copy(other.elements_, other.elements_ + other.size_, elements_);
    }
    Vector(Vector&& other)
        : elements_(other.elements_), size_(other.size_), capacity_(other.capacity_) {
        other.elements_ = new int[0];
        other.size_ = 0;
        other.capacity_ = 0;
    }

    Vector& operator=(const Vector& other) {
        Vector tmp(other);
        Swap(tmp);
        return *this;
    }
    Vector& operator=(Vector&& other) {
        Vector tmp(std::move(other));
        Swap(tmp);
        return *this;
    }

    int& operator[](size_t index) {
        return elements_[index];
    }
    const int& operator[](size_t index) const {
        return elements_[index];
    }

    void Swap(Vector& other) {
        std::swap(elements_, other.elements_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
    size_t Size() const {
        return size_;
    }
    size_t Capacity() const {
        return capacity_;
    }
    void PushBack(int element) {
        if (size_ >= capacity_) {
            Reserve((capacity_ == 0 ? 1 : 2 * capacity_));
            PushBack(element);
        } else {
            elements_[size_] = element;
            ++size_;
        }
    }
    void PopBack() {
        size_ = (size_ > 0) ? size_ - 1 : 0;
    }
    void Clear() {
        size_ = 0;
    }
    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            int* new_elements = new int[new_capacity];
            std::copy(elements_, elements_ + size_, new_elements);
            delete[] elements_;
            elements_ = new_elements;
            capacity_ = new_capacity;
        }
    }
    Iterator begin() {
        return Iterator(elements_);
    }
    Iterator end() {
        return Iterator(elements_ + size_);
    }

private:
    int* elements_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

Vector::Iterator operator+(const Vector::Iterator& it, Vector::Iterator::difference_type n) {
    Vector::Iterator copy = it;
    copy += n;
    return copy;
}
Vector::Iterator operator+(Vector::Iterator::difference_type n, const Vector::Iterator& it) {
    Vector::Iterator copy = it;
    copy += n;
    return copy;
}
Vector::Iterator operator-(const Vector::Iterator& it, Vector::Iterator::difference_type n) {
    Vector::Iterator copy = it;
    copy -= n;
    return copy;
}
Vector::Iterator operator-(Vector::Iterator::difference_type n, const Vector::Iterator& it) {
    Vector::Iterator copy = it;
    copy -= n;
    return copy;
}
Vector::Iterator::difference_type operator-(const Vector::Iterator& lhs,
                                            const Vector::Iterator& rhs) {
    return lhs.element_ptr_ - rhs.element_ptr_;
}

bool operator==(const Vector::Iterator& lhs, const Vector::Iterator& rhs) {
    return lhs.element_ptr_ == rhs.element_ptr_;
}
bool operator!=(const Vector::Iterator& lhs, const Vector::Iterator& rhs) {
    return lhs.element_ptr_ != rhs.element_ptr_;
}
bool operator>=(const Vector::Iterator& lhs, const Vector::Iterator& rhs) {
    return lhs.element_ptr_ >= rhs.element_ptr_;
}
bool operator<=(const Vector::Iterator& lhs, const Vector::Iterator& rhs) {
    return lhs.element_ptr_ <= rhs.element_ptr_;
}
bool operator>(const Vector::Iterator& lhs, const Vector::Iterator& rhs) {
    return lhs.element_ptr_ > rhs.element_ptr_;
}
bool operator<(const Vector::Iterator& lhs, const Vector::Iterator& rhs) {
    return lhs.element_ptr_ < rhs.element_ptr_;
}
