#pragma once

#include <iostream>
#include <cstddef>
#include <iterator>

template <typename T>
class List {
private:
    struct BaseNode;
    struct Node;

public:
    class Iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        Iterator() : ptr_(nullptr) {
        }
        Iterator(BaseNode* ptr) : ptr_(ptr) {
        }
        Iterator& operator++() {
            ptr_ = ptr_->next;
            return *this;
        }
        Iterator operator++(int) {
            Iterator tmp(ptr_);
            ptr_ = ptr_->next;
            return tmp;
        }

        Iterator& operator--() {
            ptr_ = ptr_->prev;
            return *this;
        }
        Iterator operator--(int) {
            Iterator tmp(ptr_);
            ptr_ = ptr_->prev;
            return tmp;
        }

        T& operator*() const {
            return reinterpret_cast<Node*>(ptr_)->value;
        }
        T* operator->() const {
            return &(reinterpret_cast<Node*>(ptr_)->value);
        }

        bool operator==(const Iterator& other) const {
            return ptr_ == other.ptr_;
        }
        bool operator!=(const Iterator& other) const {
            return ptr_ != other.ptr_;
        }

        friend void List<T>::Erase(Iterator);

    private:
        BaseNode* ptr_;
    };

    class ConstIterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        ConstIterator() : ptr_(nullptr) {
        }
        ConstIterator(BaseNode* ptr) : ptr_(ptr) {
        }
        ConstIterator& operator++() {
            ptr_ = ptr_->next;
            return *this;
        }
        ConstIterator operator++(int) {
            ConstIterator tmp(ptr_);
            ptr_ = ptr_->next;
            return tmp;
        }

        ConstIterator& operator--() {
            ptr_ = ptr_->prev;
            return *this;
        }
        ConstIterator operator--(int) {
            ConstIterator tmp(ptr_);
            ptr_ = ptr_->prev;
            return tmp;
        }

        const T& operator*() const {
            return reinterpret_cast<Node*>(ptr_)->value;
        }
        const T* operator->() const {
            return &(reinterpret_cast<Node*>(ptr_)->value);
        }

        bool operator==(const ConstIterator& other) const {
            return ptr_ == other.ptr_;
        }
        bool operator!=(const ConstIterator& other) const {
            return ptr_ != other.ptr_;
        }

    private:
        BaseNode* ptr_;
    };

    List() : root_ptr_(new BaseNode{}), size_(0) {
        root_ptr_->prev = root_ptr_;
        root_ptr_->next = root_ptr_;
    }
    List(const List& other) : List() {
        for (const T& value : other) {
            PushBack(value);
        }
    }
    List(List&& other) : root_ptr_(other.root_ptr_), size_(other.size_) {
        other.root_ptr_ = new BaseNode{};
        other.root_ptr_->prev = other.root_ptr_;
        other.root_ptr_->next = other.root_ptr_;
        other.size_ = 0;
    }
    ~List() {
        while (size_ > 0) {
            PopBack();
        }
        delete root_ptr_;
    }

    List& operator=(const List& other) {
        List tmp(other);
        Swap(tmp);
        return *this;
    }
    List& operator=(List&& other) {
        this->~List();
        root_ptr_ = other.root_ptr_;
        size_ = other.size_;
        other.root_ptr_ = new BaseNode{};
        other.root_ptr_->prev = other.root_ptr_;
        other.root_ptr_->next = other.root_ptr_;
        other.size_ = 0;
        return *this;
    }

    bool IsEmpty() const {
        return (size_ == 0);
    }
    size_t Size() const {
        return size_;
    }

    void PushBack(const T& value) {
        LinkAfter(root_ptr_->prev, new Node(value));
        ++size_;
    }
    void PushBack(T&& value) {
        LinkAfter(root_ptr_->prev, new Node(std::move(value)));
        ++size_;
    }
    void PushFront(const T& value) {
        LinkAfter(root_ptr_, new Node(value));
        ++size_;
    }
    void PushFront(T&& value) {
        LinkAfter(root_ptr_, new Node(std::move(value)));
        ++size_;
    }

    T& Front() {
        return reinterpret_cast<Node*>(root_ptr_->next)->value;
    }
    const T& Front() const {
        return reinterpret_cast<Node*>(root_ptr_->next)->value;
    }
    T& Back() {
        return reinterpret_cast<Node*>(root_ptr_->prev)->value;
    }
    const T& Back() const {
        return reinterpret_cast<Node*>(root_ptr_->prev)->value;
    }

    void PopBack() {
        auto back_ptr = reinterpret_cast<Node*>(root_ptr_->prev);
        Unlink(back_ptr);
        delete back_ptr;
        --size_;
    }
    void PopFront() {
        auto front_ptr = reinterpret_cast<Node*>(root_ptr_->next);
        Unlink(front_ptr);
        delete front_ptr;
        --size_;
    }

    void Erase(Iterator it) {
        auto ptr = reinterpret_cast<Node*>(it.ptr_);
        Unlink(ptr);
        delete ptr;
        --size_;
    }

    Iterator Begin() {
        return Iterator(root_ptr_->next);
    }
    Iterator End() {
        return Iterator(root_ptr_);
    }

    ConstIterator Begin() const {
        return ConstIterator(root_ptr_->next);
    }
    ConstIterator End() const {
        return ConstIterator(root_ptr_);
    }

private:
    struct BaseNode {
        BaseNode* prev;
        BaseNode* next;
    };

    struct Node : BaseNode {
        T value;

        Node(const T& value) : value(value) {
        }
        Node(T&& value) : value(std::move(value)) {
        }
    };

    void Swap(List& other) {
        std::swap(root_ptr_, other.root_ptr_);
        std::swap(size_, other.size_);
    }

    void Unlink(BaseNode* node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        node->prev = nullptr;
        node->next = nullptr;
    }

    void LinkAfter(BaseNode* target, BaseNode* after) {
        target->next->prev = after;
        after->next = target->next;
        target->next = after;
        after->prev = target;
    }

    BaseNode* root_ptr_;
    size_t size_;
};

template <typename T>
std::ostream& operator<<(std::ostream& out, const List<T>& list_object) {
    for (const T& value : list_object) {
        out << value << ' ';
    }
    return out;
}

template <typename T>
List<T>::Iterator begin(List<T>& list_object) {
    return list_object.Begin();
}

template <typename T>
List<T>::Iterator end(List<T>& list_object) {
    return list_object.End();
}

template <typename T>
List<T>::ConstIterator begin(const List<T>& list_object) {
    return list_object.Begin();
}

template <typename T>
List<T>::ConstIterator end(const List<T>& list_object) {
    return list_object.End();
}
