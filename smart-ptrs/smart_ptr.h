#pragma once

#include <string>

struct LinkCounter {
    int strong = 0;
    int weak = 0;
};

class WeakPtr;

class SharedPtr {
    friend WeakPtr;

public:
    SharedPtr() = default;
    SharedPtr(std::string* ptr) : obj_ptr_(ptr), counter_(new LinkCounter{1, 0}) {
    }
    SharedPtr(const SharedPtr& other) : obj_ptr_(other.obj_ptr_), counter_(other.counter_) {
        if (counter_) {
            ++counter_->strong;
        }
    }
    SharedPtr(SharedPtr&& other) : obj_ptr_(other.obj_ptr_), counter_(other.counter_) {
        other.obj_ptr_ = nullptr;
        other.counter_ = nullptr;
    }
    SharedPtr(const WeakPtr& weak_ptr);
    ~SharedPtr() {
        if (counter_ && (counter_->strong > 0)) {
            --counter_->strong;
            if (counter_->strong == 0) {
                delete obj_ptr_;
                obj_ptr_ = nullptr;
                if (counter_->weak == 0) {
                    delete counter_;
                    counter_ = nullptr;
                }
            }
        }
    }

    SharedPtr& operator=(SharedPtr other) {
        std::swap(other.obj_ptr_, obj_ptr_);
        std::swap(other.counter_, counter_);
        return *this;
    }

    std::string& operator*() const {
        return *obj_ptr_;
    }
    std::string* operator->() const {
        return obj_ptr_;
    }

    std::string* Get() const {
        return obj_ptr_;
    }

    void Reset(std::string* ptr) {
        this->~SharedPtr();
        obj_ptr_ = ptr;
        counter_ = new LinkCounter{1, 0};
    }

private:
    std::string* obj_ptr_ = nullptr;
    LinkCounter* counter_ = nullptr;
};

class WeakPtr {
    friend SharedPtr;

public:
    WeakPtr() = default;
    WeakPtr(const WeakPtr& other) : obj_ptr_(other.obj_ptr_), counter_(other.counter_) {
        if (counter_) {
            ++counter_->weak;
        }
    }
    WeakPtr(WeakPtr&& other) : obj_ptr_(other.obj_ptr_), counter_(other.counter_) {
        other.obj_ptr_ = nullptr;
        other.counter_ = nullptr;
    }
    WeakPtr(const SharedPtr& shared_ptr)
        : obj_ptr_(shared_ptr.obj_ptr_), counter_(shared_ptr.counter_) {
        if (counter_) {
            ++counter_->weak;
        }
    }
    ~WeakPtr() {
        if (counter_ && (counter_->weak > 0)) {
            --counter_->weak;
            if ((counter_->weak == 0) && (counter_->strong == 0)) {
                delete counter_;
                counter_ = nullptr;
                obj_ptr_ = nullptr;
            }
        }
    }

    WeakPtr& operator=(WeakPtr other) {
        std::swap(other.obj_ptr_, obj_ptr_);
        std::swap(other.counter_, counter_);
        return *this;
    }

    bool IsExpired() const {
        if (counter_ && (counter_->strong > 0)) {
            return false;
        }
        return true;
    }

    SharedPtr Lock() const {
        if (!IsExpired()) {
            return SharedPtr(*this);
        }
        return SharedPtr();
    }

    void Reset() {
        this->~WeakPtr();
        obj_ptr_ = nullptr;
        counter_ = nullptr;
    }

private:
    std::string* obj_ptr_ = nullptr;
    LinkCounter* counter_ = nullptr;
};

SharedPtr::SharedPtr(const WeakPtr& weak_ptr)
    : obj_ptr_(weak_ptr.obj_ptr_), counter_(weak_ptr.counter_) {
    if (weak_ptr.IsExpired()) {
        obj_ptr_ = nullptr;
        counter_ = nullptr;
    } else {
        ++counter_->strong;
    }
}
