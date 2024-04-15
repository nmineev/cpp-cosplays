#pragma once
#include <cstdlib>
#include <utility>
#include <algorithm>
#include <exception>

template <class T>
class Optional {
public:
    Optional() = default;

    Optional(const T& value) {
        new (buf_) T(value);
        has_value_ = true;
    }

    Optional(T&& value) {
        new (buf_) T(std::move(value));
        has_value_ = true;
    }

    Optional(const Optional& other) {
        if (other.has_value_) {
            new (buf_) T(*other.GetValuePtr());
            has_value_ = true;
        }
    }

    // This constructor must not change other.HasValue()
    // Just move value (if present)
    Optional(Optional&& other) {
        if (other.has_value_) {
            new (buf_) T(std::move(*other.GetValuePtr()));
            has_value_ = true;
        }
    }

    ~Optional() {
        if (has_value_) {
            GetValuePtr()->~T();
        }
        has_value_ = false;
    }

    Optional& operator=(const Optional& other) {
        if (this != &other) {
            if (other.has_value_ && has_value_) {
                T value_copy = *other.GetValuePtr();
                GetValuePtr()->~T();
                new (buf_) T(value_copy);
                has_value_ = true;
            } else if (other.has_value_ && !has_value_) {
                new (buf_) T(*other.GetValuePtr());
                has_value_ = true;
            } else {
                if (has_value_) {
                    GetValuePtr()->~T();
                }
                has_value_ = false;
            }
        }
        return *this;
    }

    // This method must not change other.HasValue()
    // Just move value (if present)
    Optional& operator=(Optional&& other) {
        if (this != &other) {
            if (other.has_value_ && has_value_) {
                T value_copy(std::move(*other.GetValuePtr()));
                GetValuePtr()->~T();
                new (buf_) T(std::move(value_copy));
                has_value_ = true;
            } else if (other.has_value_ && !has_value_) {
                new (buf_) T(std::move(*other.GetValuePtr()));
                has_value_ = true;
            } else {
                if (has_value_) {
                    GetValuePtr()->~T();
                }
                has_value_ = false;
            }
        }
        return *this;
    }

    T& operator*() & {
        return *GetValuePtr();
    }

    const T& operator*() const& {
        return *GetValuePtr();
    }

    T&& operator*() && {
        return std::move(*GetValuePtr());
    }

    T* operator->() {
        return GetValuePtr();
    }

    const T* operator->() const {
        return GetValuePtr();
    }

    T& Value() & {
        return *GetValuePtr();
    }

    const T& Value() const& {
        return *GetValuePtr();
    }

    T&& Value() && {
        return std::move(*GetValuePtr());
    }

    bool HasValue() const {
        return has_value_;
    }

    void Reset() {
        if (has_value_) {
            GetValuePtr()->~T();
            has_value_ = false;
        }
    }

private:
    alignas(T) unsigned char buf_[sizeof(T)];
    bool has_value_ = false;

    T* GetValuePtr() {
        if (!has_value_) {
            throw std::exception();
        }
        return reinterpret_cast<T*>(buf_);
    }

    const T* GetValuePtr() const {
        if (!has_value_) {
            throw std::exception();
        }
        return reinterpret_cast<const T*>(buf_);
    }
};
