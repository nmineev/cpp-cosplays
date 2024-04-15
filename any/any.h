#pragma once

#include <type_traits>
#include <concepts>
#include <utility>
#include <typeinfo>
#include <type_traits>

template <class T>
concept NotAny = !std::same_as<std::remove_cvref_t<T>, class Any>;

class Any {
    struct InnerBase;
    template <typename T>
    struct Inner;

public:
    Any() = default;

    // T&& - universal (forwarding) reference asdfasdf
    // use std::forward inside this constructor
    template <NotAny T>
    Any(T&& value) : inner_ptr_(new Inner<std::decay_t<T>>(std::forward<T>(value))) {
    }

    Any(const Any& other) {
        if (other.inner_ptr_) {
            inner_ptr_ = other.inner_ptr_->Clone();
        }
    }
    Any(Any&& other) : inner_ptr_(other.inner_ptr_) {
        other.inner_ptr_ = nullptr;
    }
    Any& operator=(const Any& other) {
        if (&other != this) {
            Any(other).Swap(*this);
        }
        return *this;
    }
    Any& operator=(Any&& other) {
        Any(std::move(other)).Swap(*this);
        return *this;
    }
    template <NotAny T>
    Any& operator=(T&& value) {
        Any(std::forward<T>(value)).Swap(*this);
        return *this;
    }
    ~Any() {
        if (inner_ptr_) {
            delete inner_ptr_;
        }
        inner_ptr_ = nullptr;
    }

    bool Empty() const {
        return !inner_ptr_;
    }

    void Clear() {
        this->~Any();
    }
    void Swap(Any& other) {
        std::swap(inner_ptr_, other.inner_ptr_);
    }

    template <class T>
    const T& GetValue() const {
        return Cast<T>();
    }

private:
    InnerBase* inner_ptr_ = nullptr;

    struct InnerBase {
        virtual ~InnerBase() {
        }
        virtual InnerBase* Clone() const = 0;
        virtual const std::type_info& Type() const = 0;
        // virtual bool IsPOD() const = 0;
        // virtual size_t Size() const = 0;
    };

    template <typename T>
    struct Inner : public InnerBase {
        Inner(const T& newval) : value_(newval) {
        }
        Inner(T&& newval) : value_(std::move(newval)) {
        }
        virtual InnerBase* Clone() const override {
            return new Inner<T>(value_);
        }
        virtual const std::type_info& Type() const override {
            return typeid(T);
        }
        T& operator*() {
            return value_;
        }
        const T& operator*() const {
            return value_;
        }
        // virtual bool IsPOD() const { return std::is_pod<_Ty>::value; }
        // virtual size_t Size() const { return sizeof(_Ty); }

    private:
        T value_;
    };

    template <typename T>
    T& Cast() {
        return *dynamic_cast<Inner<std::decay_t<T>>&>(*inner_ptr_);
    }

    template <typename T>
    const T& Cast() const {
        return *dynamic_cast<Inner<std::decay_t<T>>&>(*inner_ptr_);
    }
};
