#include <memory>
#include <initializer_list>
#include <iostream>
#include <algorithm>


template <typename KeyType>
struct AVLTree {
public:
    struct BaseNode {
        std::shared_ptr<BaseNode> left_node_ptr = nullptr;
        std::shared_ptr<BaseNode> right_node_ptr = nullptr;
        std::weak_ptr<BaseNode> parent_node_ptr;
        int8_t balance = 0;

        virtual ~BaseNode() {
        }
    };

    struct Node : public BaseNode {
        KeyType key;

        explicit Node(const KeyType& key) : key(key) {
        }
        explicit Node(KeyType&& key) : key(std::move(key)) {
        }
    };

    AVLTree() : fakeroot_ptr(std::make_shared<BaseNode>()), size(0) {
        leftmost_node_ptr = fakeroot_ptr;
    }

    AVLTree(const AVLTree& other) : AVLTree() {
        if (other.fakeroot_ptr->left_node_ptr != nullptr) {
            auto other_root_ptr = std::dynamic_pointer_cast<Node>(
                    other.fakeroot_ptr->left_node_ptr);
            auto root_ptr = std::make_shared<Node>(other_root_ptr->key);
            fakeroot_ptr->left_node_ptr = root_ptr;
            root_ptr->left_node_ptr = nullptr;
            root_ptr->right_node_ptr = nullptr;
            root_ptr->parent_node_ptr = fakeroot_ptr;
            root_ptr->balance = other_root_ptr->balance;
            DeepCopy(other_root_ptr, root_ptr);
            leftmost_node_ptr = Min(root_ptr);
            size = other.size;
        }
    }

    ~AVLTree() = default;

    AVLTree& operator=(const AVLTree& other) {
        AVLTree tmp(other);
        Swap(tmp);
        return *this;
    }

    std::shared_ptr<BaseNode> Find(const KeyType& key, bool find_lower_bound = false) {
        if (fakeroot_ptr->left_node_ptr == nullptr) {
            return fakeroot_ptr;
        }
        std::shared_ptr<Node> lower_bound_ptr = nullptr;
        auto node_ptr = std::dynamic_pointer_cast<Node>(fakeroot_ptr->left_node_ptr);
        while (node_ptr != nullptr) {
            if (key < node_ptr->key) {
                if ((lower_bound_ptr == nullptr) || (node_ptr->key < lower_bound_ptr->key)) {
                    lower_bound_ptr = node_ptr;
                }
                node_ptr = std::dynamic_pointer_cast<Node>(node_ptr->left_node_ptr);
            } else if (node_ptr->key < key) {
                node_ptr = std::dynamic_pointer_cast<Node>(node_ptr->right_node_ptr);
            } else {
                return node_ptr;
            }
        }
        if (find_lower_bound && (lower_bound_ptr != nullptr)) {
            return lower_bound_ptr;
        }
        return fakeroot_ptr;
    }

    std::shared_ptr<BaseNode> Min(std::shared_ptr<BaseNode> node_ptr) {
        while (node_ptr->left_node_ptr != nullptr) {
            node_ptr = node_ptr->left_node_ptr;
        }
        return node_ptr;
    }

    std::shared_ptr<BaseNode> Max(std::shared_ptr<BaseNode> node_ptr) {
        while (node_ptr->right_node_ptr != nullptr) {
            node_ptr = node_ptr->right_node_ptr;
        }
        return node_ptr;
    }

    std::shared_ptr<BaseNode> Next(std::shared_ptr<BaseNode> node_ptr) {
        if (node_ptr->right_node_ptr != nullptr) {
            return Min(node_ptr->right_node_ptr);
        }
        auto parent_node_ptr = node_ptr->parent_node_ptr.lock();
        while ((parent_node_ptr != nullptr) && (node_ptr == parent_node_ptr->right_node_ptr)) {
            node_ptr = parent_node_ptr;
            parent_node_ptr = parent_node_ptr->parent_node_ptr.lock();
        }
        return parent_node_ptr;
    }

    std::shared_ptr<BaseNode> Prev(std::shared_ptr<BaseNode> node_ptr) {
        if (node_ptr->left_node_ptr != nullptr) {
            return Max(node_ptr->left_node_ptr);
        }
        auto parent_node_ptr = node_ptr->parent_node_ptr.lock();
        while ((parent_node_ptr != nullptr) && (node_ptr == parent_node_ptr->left_node_ptr)) {
            node_ptr = parent_node_ptr;
            parent_node_ptr = parent_node_ptr->parent_node_ptr.lock();
        }
        return parent_node_ptr;
    }

    std::shared_ptr<Node> Insert(std::shared_ptr<Node>& new_node_ptr) {
        new_node_ptr->left_node_ptr = nullptr;
        new_node_ptr->right_node_ptr = nullptr;
        new_node_ptr->balance = 0;

        if (fakeroot_ptr->left_node_ptr == nullptr) {
            fakeroot_ptr->left_node_ptr = new_node_ptr;
            new_node_ptr->parent_node_ptr = fakeroot_ptr;
            leftmost_node_ptr = new_node_ptr;
            ++size;
            return new_node_ptr;
        }
        auto current_node_ptr = std::dynamic_pointer_cast<Node>(fakeroot_ptr->left_node_ptr);
        while (current_node_ptr != nullptr) {
            if (new_node_ptr->key < current_node_ptr->key) {
                if (current_node_ptr->left_node_ptr != nullptr) {
                    current_node_ptr = std::dynamic_pointer_cast<Node>(
                        current_node_ptr->left_node_ptr);
                } else {
                    new_node_ptr->parent_node_ptr = current_node_ptr;
                    current_node_ptr->left_node_ptr = new_node_ptr;
                    ++current_node_ptr->balance;
                    break;
                }
            } else if (current_node_ptr->key < new_node_ptr->key) {
                if (current_node_ptr->right_node_ptr != nullptr) {
                    current_node_ptr = std::dynamic_pointer_cast<Node>(
                        current_node_ptr->right_node_ptr);
                } else {
                    new_node_ptr->parent_node_ptr = current_node_ptr;
                    current_node_ptr->right_node_ptr = new_node_ptr;
                    --current_node_ptr->balance;
                    break;
                }
            } else {
                return current_node_ptr;
            }
        }
        if (leftmost_node_ptr->left_node_ptr != nullptr) {
            leftmost_node_ptr = leftmost_node_ptr->left_node_ptr;
        }
        ++size;
        Rebalance(current_node_ptr, true);
        return new_node_ptr;
    }

    std::shared_ptr<Node> Insert(const KeyType& key) {
        auto new_node_ptr = std::make_shared<Node>(key);
        return Insert(new_node_ptr);
    }

    std::shared_ptr<BaseNode> Erase(std::shared_ptr<BaseNode>& node_ptr) {
        if (node_ptr == fakeroot_ptr) {
            return fakeroot_ptr;
        }
        auto next_node_ptr = Next(node_ptr);
        if (node_ptr == leftmost_node_ptr) {
            leftmost_node_ptr = next_node_ptr;
        }
        if ((node_ptr->left_node_ptr != nullptr) && (node_ptr->right_node_ptr != nullptr)) {
            SwapNodes(node_ptr, next_node_ptr);
        }
        auto parent_node_ptr = node_ptr->parent_node_ptr.lock();
        if (parent_node_ptr->left_node_ptr == node_ptr) {
            --parent_node_ptr->balance;
            if (node_ptr->right_node_ptr != nullptr) {
                parent_node_ptr->left_node_ptr = node_ptr->right_node_ptr;
                node_ptr->right_node_ptr->parent_node_ptr = parent_node_ptr;
            } else if (node_ptr->left_node_ptr != nullptr) {
                parent_node_ptr->left_node_ptr = node_ptr->left_node_ptr;
                node_ptr->left_node_ptr->parent_node_ptr = parent_node_ptr;
            } else {
                parent_node_ptr->left_node_ptr = nullptr;
            }
        } else {
            ++parent_node_ptr->balance;
            if (node_ptr->right_node_ptr != nullptr) {
                parent_node_ptr->right_node_ptr = node_ptr->right_node_ptr;
                node_ptr->right_node_ptr->parent_node_ptr = parent_node_ptr;
            } else if (node_ptr->left_node_ptr != nullptr) {
                parent_node_ptr->right_node_ptr = node_ptr->left_node_ptr;
                node_ptr->left_node_ptr->parent_node_ptr = parent_node_ptr;
            } else {
                parent_node_ptr->right_node_ptr = nullptr;
            }
        }
        node_ptr->left_node_ptr = nullptr;
        node_ptr->right_node_ptr = nullptr;
        node_ptr->balance = 0;
        --size;
        Rebalance(parent_node_ptr, false);
        return next_node_ptr;
    }

    std::shared_ptr<BaseNode> Erase(const KeyType& key) {
        auto node_ptr = Find(key);
        if ((node_ptr == nullptr) || (node_ptr == fakeroot_ptr)) {
            return fakeroot_ptr;
        }
        return Erase(node_ptr);
    }

    size_t Size() {
        return size;
    }

    std::shared_ptr<BaseNode> Begin() {
        return leftmost_node_ptr;
    }
    std::shared_ptr<BaseNode> End() {
        return fakeroot_ptr;
    }

    void Print(std::ostream& out = std::cout) {
        if (fakeroot_ptr->left_node_ptr == nullptr) {
            return;
        }
        Print(std::dynamic_pointer_cast<Node>(fakeroot_ptr->left_node_ptr), out);
        out << '\n';
    }


private:
    static void DeepCopy(const std::shared_ptr<Node>& orig_node_ptr,
                         std::shared_ptr<Node> copy_node_ptr) {
        if (orig_node_ptr != nullptr) {
            if (orig_node_ptr->left_node_ptr != nullptr) {
                copy_node_ptr->left_node_ptr = std::make_shared<Node>(
                        std::dynamic_pointer_cast<Node>(orig_node_ptr->left_node_ptr)->key);
                copy_node_ptr->left_node_ptr->left_node_ptr = nullptr;
                copy_node_ptr->left_node_ptr->right_node_ptr = nullptr;
                copy_node_ptr->left_node_ptr->parent_node_ptr = copy_node_ptr;
                copy_node_ptr->left_node_ptr->balance = orig_node_ptr->left_node_ptr->balance;
                DeepCopy(std::dynamic_pointer_cast<Node>(orig_node_ptr->left_node_ptr),
                         std::dynamic_pointer_cast<Node>(copy_node_ptr->left_node_ptr));
            }
            if (orig_node_ptr->right_node_ptr != nullptr) {
                copy_node_ptr->right_node_ptr = std::make_shared<Node>(
                        std::dynamic_pointer_cast<Node>(orig_node_ptr->right_node_ptr)->key);
                copy_node_ptr->right_node_ptr->left_node_ptr = nullptr;
                copy_node_ptr->right_node_ptr->right_node_ptr = nullptr;
                copy_node_ptr->right_node_ptr->parent_node_ptr = copy_node_ptr;
                copy_node_ptr->right_node_ptr->balance = orig_node_ptr->right_node_ptr->balance;
                DeepCopy(std::dynamic_pointer_cast<Node>(orig_node_ptr->right_node_ptr),
                         std::dynamic_pointer_cast<Node>(copy_node_ptr->right_node_ptr));
            }
        }
    }

    static void Print(const std::shared_ptr<Node>& node_ptr, std::ostream& out) {
        if (node_ptr == nullptr) {
            out << " . ";
            return;
        }
        out << '(';
        Print(std::dynamic_pointer_cast<Node>(node_ptr->left_node_ptr), out);
        out << ") {" << node_ptr->key << ',' << static_cast<int32_t>(node_ptr->balance) << "} (";
        Print(std::dynamic_pointer_cast<Node>(node_ptr->right_node_ptr), out);
        out << ')';
    }

    static void SwapNodes(std::shared_ptr<BaseNode> lhs, std::shared_ptr<BaseNode> rhs) {
        if (lhs->left_node_ptr != nullptr) {
            lhs->left_node_ptr->parent_node_ptr = rhs;
        }
        if (rhs->left_node_ptr != nullptr) {
            rhs->left_node_ptr->parent_node_ptr = lhs;
        }
        std::swap(lhs->left_node_ptr, rhs->left_node_ptr);

        if (lhs->right_node_ptr != nullptr) {
            lhs->right_node_ptr->parent_node_ptr = rhs;
        }
        if (rhs->right_node_ptr != nullptr) {
            rhs->right_node_ptr->parent_node_ptr = lhs;
        }
        std::swap(lhs->right_node_ptr, rhs->right_node_ptr);

        auto lhs_parent_node_ptr = lhs->parent_node_ptr.lock();
        if (lhs_parent_node_ptr != nullptr) {
            if (lhs == lhs_parent_node_ptr->left_node_ptr) {
                lhs_parent_node_ptr->left_node_ptr = rhs;
            } else {
                lhs_parent_node_ptr->right_node_ptr = rhs;
            }
        }
        auto rhs_parent_node_ptr = rhs->parent_node_ptr.lock();
        if (rhs_parent_node_ptr != nullptr) {
            if (rhs == rhs_parent_node_ptr->left_node_ptr) {
                rhs_parent_node_ptr->left_node_ptr = lhs;
            } else {
                rhs_parent_node_ptr->right_node_ptr = lhs;
            }
        }
        std::swap(lhs->parent_node_ptr, rhs->parent_node_ptr);
        std::swap(lhs->balance, rhs->balance);
    }

    void Swap(AVLTree& other) {
        std::swap(fakeroot_ptr, other.fakeroot_ptr);
        std::swap(leftmost_node_ptr, other.leftmost_node_ptr);
        std::swap(size, other.size);
    }

    void RotateLeft(std::shared_ptr<BaseNode> node_a_ptr) {
        /*
           a            b
          / \          / \
         L   b   =>   a   R
            / \      / \
           M   R    L   M
        */
        auto node_b_ptr = node_a_ptr->right_node_ptr;
        node_a_ptr->right_node_ptr = node_b_ptr->left_node_ptr;
        if (node_b_ptr->left_node_ptr != nullptr) {
            node_b_ptr->left_node_ptr->parent_node_ptr = node_a_ptr;
        }
        node_b_ptr->left_node_ptr = node_a_ptr;
        node_b_ptr->parent_node_ptr = node_a_ptr->parent_node_ptr;
        if (!node_a_ptr->parent_node_ptr.expired()) {
            auto parent_node_ptr = node_a_ptr->parent_node_ptr.lock();
            if (node_a_ptr == parent_node_ptr->left_node_ptr) {
                parent_node_ptr->left_node_ptr = node_b_ptr;
            } else {
                parent_node_ptr->right_node_ptr = node_b_ptr;
            }
        }
        node_a_ptr->parent_node_ptr = node_b_ptr;

        node_a_ptr->balance += 1 + (node_b_ptr->balance < 0 ? -node_b_ptr->balance : 0);
        node_b_ptr->balance += 1 + (node_b_ptr->balance < 0 ? node_a_ptr->balance : 0);
    }

    void RotateRight(std::shared_ptr<BaseNode> node_a_ptr) {
        /*
             a            b
            / \          / \
           b   R   =>   L   a
          / \              / \
         L   M            M   R
        */
        auto node_b_ptr = node_a_ptr->left_node_ptr;
        node_a_ptr->left_node_ptr = node_b_ptr->right_node_ptr;
        if (node_b_ptr->right_node_ptr != nullptr) {
            node_b_ptr->right_node_ptr->parent_node_ptr = node_a_ptr;
        }
        node_b_ptr->right_node_ptr = node_a_ptr;
        node_b_ptr->parent_node_ptr = node_a_ptr->parent_node_ptr;
        if (!node_a_ptr->parent_node_ptr.expired()) {
            auto parent_node_ptr = node_a_ptr->parent_node_ptr.lock();
            if (node_a_ptr == parent_node_ptr->left_node_ptr) {
                parent_node_ptr->left_node_ptr = node_b_ptr;
            } else {
                parent_node_ptr->right_node_ptr = node_b_ptr;
            }
        }
        node_a_ptr->parent_node_ptr = node_b_ptr;

        node_a_ptr->balance += -1 + (node_b_ptr->balance > 0 ? -node_b_ptr->balance : 0);
        node_b_ptr->balance += -1 + (node_b_ptr->balance > 0 ? node_a_ptr->balance : 0);
    }

    void RotateRightLeft(std::shared_ptr<BaseNode>& node_ptr) {
        RotateRight(node_ptr->right_node_ptr);
        RotateLeft(node_ptr);
    }

    void RotateLeftRight(std::shared_ptr<BaseNode>& node_ptr) {
        RotateLeft(node_ptr->left_node_ptr);
        RotateRight(node_ptr);
    }

    void Rebalance(std::shared_ptr<BaseNode> node_ptr, bool on_insert) {
        while ((node_ptr != nullptr) && (node_ptr != fakeroot_ptr)) {
            if ((on_insert && (node_ptr->balance == 0))
                || (!on_insert && ((node_ptr->balance == -1) || (node_ptr->balance == 1)))) {
                break;
            }
            if (node_ptr->balance == -2) {
                if (node_ptr->right_node_ptr->balance == 1) {
                    RotateRightLeft(node_ptr);
                } else {
                    RotateLeft(node_ptr);
                }
            } else if (node_ptr->balance == 2) {
                if (node_ptr->left_node_ptr->balance == -1) {
                    RotateLeftRight(node_ptr);
                } else {
                    RotateRight(node_ptr);
                }
            } else {
                auto parent_node_ptr = node_ptr->parent_node_ptr.lock();
                if (node_ptr == parent_node_ptr->left_node_ptr) {
                    parent_node_ptr->balance += (on_insert ? 1 : -1);
                } else {
                    parent_node_ptr->balance += (on_insert ? -1 : 1);
                }
            }
            node_ptr = node_ptr->parent_node_ptr.lock();
        }
        fakeroot_ptr->balance = 0;
    }

    std::shared_ptr<BaseNode> fakeroot_ptr;
    std::shared_ptr<BaseNode> leftmost_node_ptr;
    size_t size;
};


template <typename KeyType>
class Set {
public:
    using TreeType = AVLTree<KeyType>;
    using BaseNodeType = typename AVLTree<KeyType>::BaseNode;
    using NodeType = typename AVLTree<KeyType>::Node;

    class iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = const KeyType;
        using difference_type = std::ptrdiff_t;
        using pointer = const KeyType*;
        using reference = const KeyType&;

        iterator() = default;
        iterator(const std::shared_ptr<TreeType>& tree_ptr,
                      const std::shared_ptr<BaseNodeType>& node_ptr)
            : tree_ptr_(tree_ptr), node_ptr_(node_ptr) {
        }
        iterator(const iterator& other)
            : tree_ptr_(other.tree_ptr_), node_ptr_(other.node_ptr_) {
        }
        iterator& operator++() {
            node_ptr_ = tree_ptr_->Next(node_ptr_);
            return *this;
        }
        iterator operator++(int) {
            iterator tmp_obj(*this);
            node_ptr_ = tree_ptr_->Next(node_ptr_);
            return tmp_obj;
        }

        iterator& operator--() {
            node_ptr_ = tree_ptr_->Prev(node_ptr_);
            return *this;
        }
        iterator operator--(int) {
            iterator tmp_obj(*this);
            node_ptr_ = tree_ptr_->Prev(node_ptr_);
            return tmp_obj;
        }

        const KeyType& operator*() const {
            return std::dynamic_pointer_cast<NodeType>(node_ptr_)->key;
        }
        const KeyType* operator->() const {
            return &(std::dynamic_pointer_cast<NodeType>(node_ptr_)->key);
        }

        bool operator==(const iterator& other) const {
            return node_ptr_ == other.node_ptr_;
        }
        bool operator!=(const iterator& other) const {
            return node_ptr_ != other.node_ptr_;
        }

    private:
        std::shared_ptr<TreeType> tree_ptr_ = nullptr;
        std::shared_ptr<BaseNodeType> node_ptr_ = nullptr;
    };


    Set() : tree_ptr_(std::make_shared<TreeType>()) {
    }
    template <typename ContainerIterator>
    Set(ContainerIterator first, ContainerIterator last) : Set() {
        for (auto current_it = first; current_it != last; ++current_it) {
            tree_ptr_->Insert(*current_it);
        }
    }
    explicit Set(std::initializer_list<KeyType> list) : Set(list.begin(), list.end()) {
    }
    Set(const Set& other) : tree_ptr_(std::make_shared<TreeType>(*other.tree_ptr_)) {
    }
    ~Set() = default;

    Set& operator=(const Set& other) {
        Set tmp(other);
        std::swap(tree_ptr_, tmp.tree_ptr_);
        return *this;
    }

    size_t size() const {
        return tree_ptr_->Size();
    }
    size_t empty() const {
        return tree_ptr_->Size() == 0;
    }

    void insert(const KeyType& key) {
        tree_ptr_->Insert(key);
    }

    void erase(const KeyType& key) {
        tree_ptr_->Erase(key);
    }

    iterator begin() const {
        return iterator(tree_ptr_, tree_ptr_->Begin());
    }
    iterator end() const {
        return iterator(tree_ptr_, tree_ptr_->End());
    }

    iterator find(const KeyType& key) const {
        return iterator(tree_ptr_, tree_ptr_->Find(key));
    }
    iterator lower_bound(const KeyType& key) const {
        return iterator(tree_ptr_, tree_ptr_->Find(key, true));
    }

    template <typename KT>
    friend std::ostream& operator<<(std::ostream& out, const Set<KT>& set);

private:
    std::shared_ptr<TreeType> tree_ptr_;
};


template <typename KeyType>
std::ostream& operator<<(std::ostream& out, const Set<KeyType>& set) {
    set.tree_ptr_->Print(out);
    return out;
}
