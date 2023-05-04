#pragma once
#include <functional>
#include <memory>
#include <utility>
#include <queue>
#include <algorithm>
#include <cassert>

#include "iterator_facade.h"

template<typename T>
constexpr std::size_t avg_print_size{1};

template<>
constexpr std::size_t avg_print_size<int>{2};

template<typename T, 
         typename Compare = std::less<T>, 
         typename Allocator = std::allocator<T>>
class avl_tree
{
    struct move_construct_tag{};
    struct copy_construct_tag{};

    struct Node
    {
        Compare & comparator;
        Node *parent{nullptr};
        Node *left{nullptr};
        Node *right{nullptr};
        std::size_t height{0};
        T value{};

        Node(Compare & comp, const T & element, Node *p = nullptr, Node *lt = nullptr, Node *rt = nullptr, std::size_t h = 0) :
            comparator{comp},
            parent{p},
            left{lt},
            right{rt},
            height{h},
            value{element}
        {}

        Node(Compare & comp, T && element, Node *p = nullptr, Node *lt = nullptr, Node *rt = nullptr, std::size_t h = 0) :
            comparator{comp},
            parent{p},
            left{lt},
            right{rt},
            height{h},
            value{std::move(element)}
        {}

        Node(const Node &) = default;
        Node(Node &&) = default;
        Node & operator=(const Node &) = default;
        Node & operator=(Node &&) = default;

        bool operator==(const Node & rhs) const
        {
            return parent == rhs.parent && left == rhs.left && right == rhs.right &&
                height == height && (!comparator(value, rhs.value) && !comparator(rhs.value, value));
        }
    };

    template<bool>
    friend class node_iterator;

    template<bool Const>
    struct node_iterator : iterator_facade<node_iterator<Const>, 
                                           std::conditional_t<Const, const typename avl_tree::value_type, typename avl_tree::value_type>,
                                           std::bidirectional_iterator_tag>
    {
        using base = iterator_facade<node_iterator<Const>, 
                                           std::conditional_t<Const, const typename avl_tree::value_type, typename avl_tree::value_type>,
                                           std::bidirectional_iterator_tag>;

        using node_ptr = std::conditional_t<Const, typename avl_tree::const_pointer, typename avl_tree::pointer>;

        node_iterator(avl_tree & tree, Node *node = nullptr) :
            parent_tree{tree},
            current_node{node}
        {}

        auto & dereference() const{
            return current_node->value;
        }

        void increment() {
            current_node = parent_tree.next(current_node);
        }

        void decrement() {
            if (nullptr == current_node){
                current_node = parent_tree.findMax();
            }
            current_node = parent_tree.prev(current_node);
        }

        template<bool C>
        bool equals(const node_iterator<C> & other) const{
            if (nullptr == current_node){ 
                return nullptr == other.current_node;
            }

            if (nullptr != other.current_node){
               return *current_node == *other.current_node;
            }

            return false;
        }

        node_ptr current_node{nullptr};
        avl_tree & parent_tree;
    };

public:
    using value_type = std::decay_t<T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using key_compare = Compare;
    using allocator_type = Allocator;
    using reference = value_type&;
    using const_reference = const value_type&;
    using node = Node;
    using node_allocator = typename std::allocator_traits<allocator_type>::rebind_alloc<node>;
    using allocator_traits = std::allocator_traits<node_allocator>;
    using pointer = typename allocator_traits::pointer;
    using const_pointer = typename allocator_traits::const_pointer;
    using iterator = node_iterator<false>;
    using const_iterator = node_iterator<true>;

public:
    avl_tree() = default;
    avl_tree(const Allocator & alloc) : m_allocator{alloc} {}

    avl_tree(avl_tree&& rhs) : 
        m_size{rhs.m_size},
        m_allocator{rhs.m_allocator},
        m_comparator{std::move(rhs.m_comparator)}
    {
        m_root = std::exchange(rhs.m_root, nullptr);
    }

    avl_tree(const avl_tree & rhs) : 
        m_allocator{rhs.m_allocator}
    {
        constructFromTree(rhs.m_root, copy_construct_tag{});
    }

    avl_tree &operator=(avl_tree&& rhs)
    {
        clear();
        m_comparator = std::move(rhs.m_comparator);
        constexpr bool pocma = allocator_traits::propagate_on_container_move_assignment::value;
        if constexpr (pocma){
            // our allocator type is not sticky
            m_allocator = rhs.m_allocator;
            m_root = std::exchange(rhs.m_root, nullptr);
        }
        else if (m_allocator == rhs.m_allocator){
            // sticky allocator, but equivalent to the other allocator type
            m_root = std::exchange(rhs.m_root, nullptr);
        }
        else{
            // non propagating allocator; non equivalent
            if (rhs.m_root){
                constructFromTree(rhs.m_root, move_construct_tag{});
                rhs.clear();
            }
        }
    }

    avl_tree &operator=(const avl_tree & rhs)
    {
        clear();
        m_comparator = rhs.m_comparator;

        constexpr bool pocca = allocator_traits::propagate_on_container_copy_assignment::value;
        if constexpr (pocca){
            m_allocator = rhs.m_allocator;
        }

        constructFromTree(rhs.m_root, copy_construct_tag{});
    }

    void swap(avl_tree & rhs) {
        constexpr bool pocs = allocator_traits::propagate_on_container_swap::value;
        using std::swap;
        swap(m_comparator, rhs.m_comparator);
        swap(m_size, rhs.m_size);

        if constexpr (pocs) {
            swap(m_allocator, rhs.m_allocator);
            swap(m_root, rhs.m_root);
        }
        else if (m_allocator == rhs.m_allocator) {
            swap(m_root, rhs.m_root);
        }
        else {
            auto temp = std::move(*this);
            *this = std::move(rhs);
            rhs = std::move(temp);
        }
    }

    ~avl_tree() { clear(); }

public:
    iterator begin() noexcept { return iterator(*this, findMin(m_root)); }
    const_iterator begin() const noexcept { return iterator(*this, findMin(m_root)); }
    const_iterator cbegin() const noexcept { return begin(); }
    
    iterator end() noexcept { return iterator(*this); }
    const_iterator end() const noexcept { return iterator(*this); }
    const_iterator cend() const noexcept { return end(); }

    iterator find(const_reference element) { return iterator(*this, find(element, m_root, m_comparator)); }
    const_iterator find(const_reference element) const { return const_iterator(*this, find(element, m_root, m_comparator)); }

    iterator findMin() { return iterator(*this, findMin(m_root)); }
    const_iterator findMin() const { return const_iterator(*this, findMin(m_root)); }

    iterator findMax() { return iterator(*this, findMax(m_root)); }
    const_iterator findMax() const { return const_iterator(*this, findMax(m_root)); }

public:
    void insert(value_type x)
    {
        insert(std::move(x), nullptr, m_root);
    }

    void remove(const_reference x)
    {
        remove(x, m_root);
    }

    void clear() { cleanup(m_root); }

    bool contains(const_reference x) const
    {
        return contains(x, m_root);
    }

    bool empty() const { return 0 == m_size; }
    size_type size() const { return m_size; }

    void print() const
    {
        std::cout << "avl_tree::print()\n";
        auto p_node = findMin(m_root);
    
        while (nullptr != p_node)
        {
            p_node = next(p_node, m_comparator);
        }
        std::cout << "\n";
    }

    void printReverse() const
    {
        std::cout << "avl_tree::print()\n";
        auto p_node = findMax(m_root);
    
        while (nullptr != p_node)
        {
            p_node = prev(p_node, m_comparator);
        }

        std::cout << "avl_tree::print() root value: " << m_root->value << "\n";

        std::cout << "\n";
    }

protected:
    template<typename... Args>
    pointer allocate(Args&&... args) {
        std::cout << "avl_tree::allocate()\n";
        pointer p = allocator_traits::allocate(m_allocator, 1);
        try{
            std::cout << "avl_tree::allocate() constructing object in allocated memory\n";
            allocator_traits::construct(m_allocator, p, std::forward<Args>(args)...);
        }
        catch(...){
            std::cout <<"avl_tree::allocate() exception catched\n";
            allocator_traits::deallocate(m_allocator, p, 1);
            p = nullptr;
            throw;
        }
        return p;
    }

    void clear(pointer & p) noexcept {
        if (p){
            allocator_traits::destroy(m_allocator, p);
            allocator_traits::deallocate(m_allocator, p, 1);
            p = nullptr;
        }
    }

    void cleanup(pointer & p_node) {
        if (nullptr == p_node) {
            return;
        }

        cleanup(p_node->left);
        cleanup(p_node->right);
        clear(p_node);
        --m_size;
    }

    void updateHeight(pointer & p_node) {
        p_node->height = std::max(height(p_node->left), height(p_node->right)) + 1;
        std::cout << "avl_tree::updateHeight() node value: " << p_node->value << "; height: " << p_node->height <<"\n";
    }

    void balance(pointer & p_node){
        std::cout << "avl_tree::balance()\n";
        if (nullptr == p_node){
            return;
        }

        std::cout << "avl_tree::balance() current node value: " << p_node->value << "\n";
        if (height(p_node->left) - height(p_node->right) > MAX_ALLOWED_IMBALANCE){
            std::cout << "avl_tree::balance() left side is unbalanced\n";
            if (height(p_node->left->left) >= height(p_node->left->right)) {
                rotateWithLeftChild(p_node);
            }
            else{
                doubleRotateWithLeftChild(p_node);
            }
        }
        else if (height(p_node->right) - height(p_node->left) > MAX_ALLOWED_IMBALANCE){
            std::cout << "avl_tree::balance() right side is unbalanced\n";
            if (height(p_node->right->right) >= height(p_node->right->left)) {
                rotateWithRightChild(p_node);
            }
            else{
                doubleRotateWithRightChild(p_node);
            }
        }

        updateHeight(p_node);
    }

    void updateParent(pointer p_node, pointer p_parent)
    {
        if (nullptr != p_node)
        {
            p_node->parent = p_parent;
        }
    }

    void rotateWithLeftChild(pointer & p_k2)
    {
        std::cout << "avl_tree::rotateWithLeftChild()\n";
        pointer p_k1 = p_k2->left;
        if (nullptr != p_k1){
            std::cout << "avl_tree::rotateWithRightChild() k2 = " << p_k2->value << " k1 = " << p_k1->value << "\n";
        }
        p_k2->left = p_k1->right;
        updateParent(p_k2->left, p_k2);
        updateParent(p_k1, p_k2->parent);
        p_k1->right = p_k2;
        updateParent(p_k2, p_k1);

        updateHeight(p_k2);
        updateHeight(p_k1);
        p_k2 = p_k1;
    }

    void rotateWithRightChild(pointer & p_k1)
    {
        pointer p_k2 = p_k1->right;
        if (nullptr != p_k2){
            std::cout << "avl_tree::rotateWithRightChild() k1 = " << p_k1->value << " k2 = " << p_k2->value << "\n";
        }
        p_k1->right = p_k2->left;
        updateParent(p_k1->right, p_k1);
        updateParent(p_k2, p_k1->parent);
        p_k2->left = p_k1;
        updateParent(p_k1, p_k2);

        updateHeight(p_k1);
        updateHeight(p_k2);
        p_k1 = p_k2;
    }

    void doubleRotateWithLeftChild(pointer & p_k3)
    {
        std::cout << "avl_tree::doubleRotateWithLeftChild()\n";
        rotateWithRightChild(p_k3->left);
        rotateWithLeftChild(p_k3);
    }

    void doubleRotateWithRightChild(pointer & p_k1)
    {
        std::cout << "avl_tree::doubleRotateWithRightChild()\n";
        rotateWithLeftChild(p_k1->right);
        rotateWithRightChild(p_k1);
    }

    template<typename U> 
    void insert(U && element, pointer p_parent, pointer & p_node){
        std::cout << "avl_tree::insert()  " << element << "\n";

        if (nullptr == p_node){
            allocateNode(p_node, std::forward<U>(element), p_parent);
        }
        else if (m_comparator(element, p_node->value)) {
            insert(std::forward<U>(element), p_node, p_node->left);
        }
        else if (m_comparator(p_node->value, element)){
            insert(std::forward<U>(element), p_node, p_node->right);
        }

        balance(p_node);
        std::cout << "avl_tree::insert() node value: " << p_node->value 
                  << "; height: " << p_node->height << "\n";
    }

    bool contains(const_reference element, pointer p_node) const
    {
        return nullptr != find(element, p_node, m_comparator);
    }

    template<typename P>
    static P parent(P p_node) { return (nullptr == p_node) ? nullptr : p_node->parent; }

    // navigateToAncestor will always go on the path from the current node to root
    template<typename P, typename Comparator>
    static P navigateToAncestor(P p_node, Comparator & comparator, bool negate)
    {
        P p_ancestor{p_node};

        static auto comp = [&comparator] (const auto & lhs, const auto & rhs, bool negate){
            return negate ? !(comparator(lhs, rhs)) : (comparator(lhs, rhs));
        };

        do {
            p_ancestor = parent(p_ancestor);
            if (nullptr == p_ancestor){
                break;
            }
            std::cout << "avl_tree::navigateToAncestor() parent: " << p_ancestor->value << "\n";
        }
        while (comp(p_ancestor->value, p_node->value, negate));

        if (nullptr != p_ancestor){
            std::cout << "avl_tree::navigateToAncestor() found: " << p_ancestor->value << "\n";
        }
        else
        {
            std::cout << "avl_tree::smallestLargestAncestor() null ancestor found\n";
        }

        return p_ancestor;
    }

    template<typename P, typename Comparator>
    static P nextBiggerParent(P p_node, Comparator & comparator)
    {
        return navigateToAncestor(p_node, comparator, false);
    }

    template<typename P, typename Comparator>
    static P prevSmallerParent(P p_node, Comparator & comparator)
    {
        return navigateToAncestor(p_node, comparator, true);
    }

    template<typename P, typename Comparator>
    static P find(const_reference element, P p_node, const Comparator & comparator)
    {
        if (nullptr == p_node){
            return nullptr;
        }

        if (comparator(element, p_node->value)){
            return find(element, p_node->left, comparator);
        }
        else if (comparator(p_node->value, element)){
            return find(element, p_node->right, comparator);
        }
        return p_node;
    }

    template<typename P>
    static P findMin(P p_node) 
    {
       while (nullptr != p_node) 
       {
           if (nullptr == p_node->left){
               std::cout << "avl_tree::findMin() found: " << p_node->value << "\n";
               return p_node;
           }
           p_node = p_node->left;
       }
       return nullptr;
    }

    template<typename P>
    static P findMax(P p_node)
    {
        if (nullptr != p_node)
        {
            if (nullptr != p_node->right){
                return findMax(p_node->right);
            }
               std::cout << "avl_tree::findMax() found: " << p_node->value << "\n";
            return p_node;
        }
        return p_node;
    }


    template<typename P, typename Comparator>
    static P next(P p_node, Comparator & comparator){
        if (nullptr == p_node)
        {
            return nullptr;
        }
        // 1. go to the smallest element of the right subtree
        std::cout << "avl_tree::next() current node: " << p_node->value << "\n";
        P p_next{nullptr};
        if (p_next = findMin(p_node->right); nullptr == p_next)
        {
            // 2. go to the smallest Ancestor larger than the current node
            p_next = nextBiggerParent(p_node, comparator);
        }
        return p_next;
    }

    template<typename P, typename Comparator>
    static P prev(P p_node, Comparator & comparator){
        if (nullptr == p_node)
        {
            return nullptr;
        }
        // 1. go to largest element of the left subtree
        P p_prev{nullptr};
        std::cout << "avl_tree::prev() current node: " << p_node->value << "\n";
        if (p_prev = findMax(p_node->left); nullptr == p_prev)
        {
            // 2. go to the prev smaller parent
            p_prev = prevSmallerParent(p_node, comparator);
        }
        return p_prev;
    }

   pointer removeMin(pointer & p_node)
   {
       if (nullptr != p_node)
       {
           if (nullptr != p_node->left){
               return removeMin(p_node->left);
           }
           else{
               pointer min_node = p_node;
               p_node = min_node->right;
               p_node->parent = min_node->parent;
               min_node->right = nullptr;
               return min_node;
           }
       }

       balance(p_node);
       return nullptr;
   }

   void remove(const_reference x, pointer & p_node)
   {
       std::cout << "avl_tree::remove() element: " << x <<"\n";
       if (nullptr == p_node) {
           return;
       }

       if (m_comparator(x, p_node->value))
       {
           std::cout << "avl_tree::remove() element is smaller than current value " << p_node->value << "\n";
           remove(x, p_node->left);
       }
       else if (m_comparator(p_node->value, x))
       {
           std::cout << "avl_tree::remove() element is bigger than current value " << p_node->value << "\n";
           remove(x, p_node->right);
       }
       else //element found
       {
           std::cout << "avl_tree::remove() element is equal to current value " << p_node->value << "\n";
           // both children are not null
           pointer removable = p_node;
           if (nullptr != p_node->right && nullptr != p_node->left)
           {
               pointer replacement = removeMin(removable->right);
               p_node = replacement;
               if (nullptr != replacement){
                   replacement->parent = removable->parent;
                   replacement->left = removable->left;
                   replacement->right = removable->right;
               }
           }
           else {
               p_node = (nullptr != p_node->left) ? p_node->left : p_node->right;
               if (nullptr != p_node){
                   p_node->parent = removable->parent;
               }
           }
           removable->parent = nullptr;
           removable->left = nullptr;
           removable->right = nullptr;
           cleanup(removable);
       }

       balance(p_node);
   }

   void constructFromTree(const_pointer rhs_node, pointer & p_node, copy_construct_tag tag)
   {
       if (nullptr == rhs_node){
           return;
       }
       allocateNode(p_node, rhs_node->value, rhs_node->parent);
       constructFromTree(rhs_node->left, p_node->left, tag);
       constructFromTree(rhs_node->right, p_node->right, tag);

       balance(p_node);
   }

   void constructFromTree(pointer rhs_node, pointer & p_node, move_construct_tag tag)
   {
       if (nullptr == rhs_node){
           return;
       }
       allocateNode(p_node, std::move(rhs_node->value), rhs_node->parent);
       constructFromTree(rhs_node->left, p_node->left, tag);
       constructFromTree(rhs_node->right, p_node->right, tag);

       balance(p_node);
   }

   template<typename... Args>
   void allocateNode(pointer & p_node, Args&&... args){
       p_node = allocate(m_comparator, std::forward<Args>(args)...);
       if (nullptr != p_node){
           ++m_size;
       }
   }

   long height(const_pointer p_node)
   {
       long h =  (nullptr == p_node) ? -1 : p_node->height;
       std::cout << "avl_tree::height() node height: " << h << "\n";
       return h;
   }

    const_pointer next(const_pointer p_node) const
    {
        return next(p_node, m_comparator);
    }

    pointer next(pointer p_node) 
    {
        return next(p_node, m_comparator);
    }

    const_pointer prev(const_pointer p_node) const
    {
        return prev(p_node, m_comparator);
    }

    pointer prev(pointer p_node) 
    {
        return prev(p_node, m_comparator);
    }

protected:
   static constexpr long MAX_ALLOWED_IMBALANCE{1};

private:
    size_type m_size{0};
    Compare m_comparator{};
    node *m_root{nullptr};
    node_allocator m_allocator{};
};
