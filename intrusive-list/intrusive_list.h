#pragma once
#include <iostream>
#include <algorithm>
#include <cstddef>
#include <type_traits>

class ListHook {
public:
    bool IsLinked() const {
        return (next_) || (prev_);
    }
    void Unlink() {
        if (prev_) {
            prev_->next_ = next_;
        }
        if (next_) {
            next_->prev_ = prev_;
        }
        next_ = nullptr;
        prev_ = nullptr;
    }

    ListHook(const ListHook&) = delete;
    ListHook& operator=(const ListHook&) = delete;

protected:
    ListHook() = default;

    // Must unlink element from list
    virtual ~ListHook() {
        Unlink();
    }

    // that helper function might be useful
    void LinkBefore(ListHook* other) {
        if (other->prev_) {
            other->prev_->next_ = this;
        }
        prev_ = other->prev_;
        other->prev_ = this;
        next_ = other;
    }

    template <class T>
    friend class List;

private:
    ListHook* next_ = nullptr;
    ListHook* prev_ = nullptr;
};

template <class T>
class List {
public:
    class Iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        Iterator() : ptr_(nullptr) {
        }
        Iterator(ListHook* ptr) : ptr_(ptr) {
        }
        Iterator& operator++() {
            ptr_ = ptr_->next_;
            return *this;
        }
        Iterator operator++(int) {
            Iterator tmp(ptr_);
            ptr_ = ptr_->next_;
            return tmp;
        }

        Iterator& operator--() {
            ptr_ = ptr_->prev_;
            return *this;
        }
        Iterator operator--(int) {
            Iterator tmp(ptr_);
            ptr_ = ptr_->prev_;
            return tmp;
        }

        T& operator*() const {
            return *dynamic_cast<T*>(ptr_);
        }
        T* operator->() const {
            return dynamic_cast<T*>(ptr_);
        }

        bool operator==(const Iterator& other) const {
            return ptr_ == other.ptr_;
        }
        bool operator!=(const Iterator& other) const {
            return ptr_ != other.ptr_;
        }

    private:
        ListHook* ptr_;
    };

    List() {
        root_.next_ = &root_;
        root_.prev_ = &root_;
    }
    List(const List&) = delete;
    List(List&& other) {
        root_.LinkBefore(&other.root_);
        other.root_.Unlink();
        other.root_.next_ = &other.root_;
        other.root_.prev_ = &other.root_;
    }

    // must unlink all elements from list
    ~List() {
        while (!IsEmpty()) {
            PopBack();
        }
        root_.next_ = &root_;
        root_.prev_ = &root_;
    }

    List& operator=(const List&) = delete;
    List& operator=(List&& other) {
        while (!IsEmpty()) {
            PopBack();
        }
        root_.next_ = &root_;
        root_.prev_ = &root_;
        root_.LinkBefore(&other.root_);
        other.root_.Unlink();
        other.root_.next_ = &other.root_;
        other.root_.prev_ = &other.root_;
        return *this;
    }

    bool IsEmpty() const {
        return root_.next_ == &root_;
    }
    // this method is allowed to be O(n)
    size_t Size() const {
        size_t size = 0;
        ListHook* curr = root_.next_;
        while (curr != &root_) {
            curr = curr->next_;
            ++size;
        }
        return size;
    }

    // note that IntrusiveList doesn't own elements,
    // and never copies or moves T
    void PushBack(T* elem) {
        elem->LinkBefore(&root_);
    }
    void PushFront(T* elem) {
        elem->LinkBefore(root_.next_);
    }

    T& Front() {
        return *dynamic_cast<T*>(root_.next_);
    }
    const T& Front() const {
        return *dynamic_cast<T*>(root_.next_);
    }

    T& Back() {
        return *dynamic_cast<T*>(root_.prev_);
    }
    const T& Back() const {
        return *dynamic_cast<T*>(root_.prev_);
    }

    void PopBack() {
        if (!IsEmpty() && root_.prev_) {
            root_.prev_->Unlink();
        }
    }
    void PopFront() {
        if (!IsEmpty() && root_.next_) {
            root_.next_->Unlink();
        }
    }

    Iterator Begin() {
        return Iterator(root_.next_);
    }
    Iterator End() {
        return Iterator(&root_);
    }

    // complexity of this function must be O(1)
    Iterator IteratorTo(T* element) {
        return Iterator(element);
    }

private:
    ListHook root_;
};

template <class T>
List<T>::Iterator begin(List<T>& list) {
    return list.Begin();
}

template <class T>
List<T>::Iterator end(List<T>& list) {
    return list.End();
}
