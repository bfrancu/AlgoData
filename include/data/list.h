#pragma once
#include <memory>
#include <algorithm>
#include <type_traits>
#include <cstdio>
#include <iostream>

#include "iterator_facade.h"
#include "traits.h"
#include "tools.h"

template<typename T>
struct object_storage
{
    template<typename... Args>
    void construct(Args &&... args)
    {
        new(storage) T(std::forward<Args>(args)...);
    }
            
    T *get() { return std::launder(reinterpret_cast<T*>(storage)); }
    uint8_t storage[sizeof(T)];
};

namespace ds
{
template<typename T,
         typename Allocator = std::allocator<T>>
class list
{
    static_assert(!ts::is_reference_v<T>, "list error: type reference not allowed");

    enum class IteratorOperation
    {
        InsertBefore,
        InsertAfter,
        EraseAt
    };

    template<IteratorOperation Op>
    struct wrap{};

    struct node : object_storage<T>
    {
        using size_type = typename list::size_type;
        using value_type = typename list::value_type;
        using node_pointer = typename list::node_pointer;

        node_pointer prev{nullptr};
        node_pointer next{nullptr};
//        T value;
        size_type id;

        node(size_type internalId, node_pointer pPrev = nullptr, node_pointer pNext = nullptr) :
            prev{pPrev},
            next{pNext},
            id{internalId}
        {}

        /*
        node(const value_type & val, size_type internalId, node_pointer pPrev = nullptr, node_pointer pNext = nullptr) :
            prev{pPrev},
            next{pNext},
            value{val},
            id{internalId}
        {}

        node(value_type && val, size_type internalId, node_pointer pPrev = nullptr, node_pointer pNext = nullptr) :
            prev{pPrev},
            next{pNext},
            value{std::move(val)},
            id{internalId}
        {}
        */

        friend bool operator==(const node & lhs, const node & rhs)
        {
            return lhs.id == rhs.id && lhs.next == rhs.next && lhs.prev == rhs.prev;
        }
    };

    template<bool Const>
    class node_iterator : public iterator_facade<node_iterator<Const>,
                                                 std::conditional_t<Const, const typename list::value_type, typename list::value_type>,
                                                 std::bidirectional_iterator_tag>
    {
        using node_ptr = typename list::node_pointer;

        friend list;
        friend iterator_facade<node_iterator<Const>, 
                               std::conditional_t<Const, const typename list::value_type, typename list::value_type>,
                               std::bidirectional_iterator_tag>;

    protected:
        node_iterator(node_ptr node = nullptr) :
            current_node{node}
        {}

        template<bool C>
        bool equals(const node_iterator<C> & other) const
        {
            if (nullptr == current_node) {
                return nullptr == other.current_node;
            }

            if (nullptr != other.current_node) {
                return current_node == other.current_node;
            }

            return false;
        }

        auto & dereference() const
        {
            return *(current_node->get());
        }

        void increment()
        {
            current_node = current_node->next;
        }

        void decrement()
        {
            current_node = current_node->prev;
        }

    private:
        node_ptr current_node;
    };

    using node_allocator = typename std::allocator_traits<Allocator>::rebind_alloc<node>;
    using node_alloc_traits = std::allocator_traits<node_allocator>;
    using node_pointer = typename node_alloc_traits::pointer;
    using const_node_pointer = typename node_alloc_traits::const_pointer;

public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits<allocator_type>::pointer;
    using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
    using iterator = node_iterator<false>;
    using const_iterator = node_iterator<true>;

public:
    ~list()
    {
        clearHeadToTail();
    }

    list(const allocator_type & alloc = {}) :
        m_alloc{alloc}
    {
        buildSentinelNodes();
    }

    list(std::initializer_list<value_type> ilist, const allocator_type & alloc = {}) :
        m_alloc{alloc}
    {
        buildSentinelNodes();
        insert(end(), ilist);
    }

    list(const list & other) :
        m_alloc{other.m_alloc}
    {
        buildSentinelNodes();
        insert(end(), other.begin(), other.end());
    }

    list(list && other) :
        m_id{other.m_id},
        m_size{other.m_size},
        m_alloc{other.m_alloc}
    {
        m_header = std::exchange(other.m_header, nullptr);
        m_tail = std::exchange(other.m_tail, nullptr);
        other.m_size = 0;
    }

    list & operator=(const list & other)
    {
        clearHeadToTail();
        constexpr bool pocca{std::allocator_traits<node_allocator>::propagate_on_container_copy_assignment::value};
        if constexpr (pocca){
            m_alloc = other.m_alloc;
        }

        buildSentinelNodes();
        insert(end(), other.begin(), other.end());
        return *this;
    }

    list & operator=(list && other)
    {
       clearHeadToTail();
       m_id = other.m_id;
       constexpr bool pocma{std::allocator_traits<node_allocator>::propagate_on_container_move_assignment::value};
       if constexpr (pocma){
           std::cout << "list::operator=(list&&) pocma\n";
           m_alloc = other.m_alloc;
           m_header = std::exchange(other.m_header, nullptr);
           m_tail = std::exchange(other.m_tail, nullptr);
           m_size = std::exchange(other.m_size, 0);
       }
       else if (m_alloc == other.m_alloc){
           m_header = std::exchange(other.m_header, nullptr);
           m_tail = std::exchange(other.m_tail, nullptr);
           m_size = std::exchange(other.m_size, 0);
       }
       else{
           buildSentinelNodes();
           insert(end(), other.begin(), other.end());
           other.clearHeadToTail();
       }
       return *this;
    }

    void swap(list & other)
    {
        constexpr bool poccs{std::allocator_traits<node_allocator>::propagate_on_container_swap::value};
        using std::swap;
        swap(m_size, other.m_size);
        swap(m_id, other.m_id);
        if constexpr (poccs){
            swap(m_alloc, other.m_alloc);
            swap(m_header, other.m_header);
            swap(m_tail, other.m_tail);
        }
        else if (m_alloc == other.m_alloc){
            swap(m_header, other.m_header);
            swap(m_tail, other.m_tail);
        }
        else{
            auto temp = std::move(*this);
            *this = std::move(other);
            other = std::move(temp);
        }
    }

public:
    iterator begin() noexcept { return iterator(frontNode()); }
    const_iterator begin() const noexcept { return const_iterator(frontNode()); }

    iterator end() noexcept { return iterator(m_tail); }
    const_iterator end() const noexcept { return const_iterator(m_tail); }

    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }

    size_type size() { return m_size; }
    bool empty() const { return 0 == m_size; }

    reference front()
    {
        return *(frontNode()->get());
    }

    const_reference front() const
    {
        return *(frontNode()->get());
    }

    reference back()
    {
        return *(backNode()->get());
    }
    
    const_reference back() const
    {
        return *(backNode()->get());
    }

    template<typename U>
    void push_back(U && val)
    {
        pushBack(std::forward<U>(val));
    }

    void pushBack(const value_type & value)
    {
//        puts("list: pushBack()");
        insertBefore(m_tail, value);
    }

    void pushBack(value_type && value)
    {
        insertBefore(m_tail, std::move(value));
    }

    void pushFront(const value_type & value)
    {
        inserAfter(m_header, value);
    }

    void pushFront(value_type && value)
    {
        insertAfter(m_header, std::move(value));
    }

    template<typename InputIt>
    void insert(iterator pos, InputIt first, InputIt last)
    {
        if (!validate(pos, wrap<IteratorOperation::InsertBefore>{}))
        {
            return;
        }

        if (first == last)
        {
            return;
        }

        for (auto it = first; it != last; ++it)
        {
            insertBefore(pos.current_node, *it);
        }
    }
    
    void insert(iterator pos, std::initializer_list<value_type> ilist)
    {
        if (!validate(pos, wrap<IteratorOperation::InsertBefore>{}))
        {
            return;
        }

        for (auto it = ilist.begin(); it != ilist.end(); ++it)
        {
            insertBefore(pos.current_node, *it);
        }
    }

    void insert(iterator pos, const value_type & value)
    {
        if (!validate(pos, wrap<IteratorOperation::InsertBefore>{}))
        {
            return;
        }
        insertBefore(pos.current_node, value);
    }

    void insert(iterator pos, value_type && value)
    {
        if (!validate(pos, wrap<IteratorOperation::InsertBefore>{}))
        {
            return;
        }
        insertBefore(pos.current_node, std::move(value));
    }

    iterator erase(iterator pos)
    {
        if (!empty() && !validate(pos, wrap<IteratorOperation::EraseAt>{}))
        {
            return end();
        }
        return iterator(eraseAt(pos.current_node));
    }

    void popBack()
    {
        if (!empty())
        {
            eraseAt(backNode());
        }
    }

    void popFront()
    {
        if (!empty()){
            eraseAt(frontNode());
        }
    }

    void clear()
    {
       if (empty()){
           return;
       }

       auto p_next = frontNode();
       for(auto p_node = p_next; p_node != m_tail && nullptr != p_node; p_node = p_next)
       {
           p_next = p_node->next;
           clear(p_node);
           --m_size;
       }
    }
   
protected:
    template<typename... Args>
    node_pointer create(size_type internalId, node_pointer pPrev,
                        node_pointer pNext, Args&&... args) 
    {
        node_pointer p = node_alloc_traits::allocate(m_alloc, 1);
        try{
            //std::cout << "list::create() constructing object in allocated memory\n";
            node *raw_p = static_cast<node*>(p);
            node_alloc_traits::construct(m_alloc, raw_p, internalId, pPrev, pNext);
            if constexpr (sizeof... (Args) > 0)
            {
                p->construct(std::forward<Args>(args)...);
            }
        }
        catch(...){
//            std::cout <<"list::create() exception catched\n";
            node_alloc_traits::deallocate(m_alloc, p, 1);
            p = nullptr;
            throw;
        }
        return p;
    }

    void clear(node_pointer p)
    {
        node *raw_p = static_cast<node*>(p);
        node_alloc_traits::destroy(m_alloc, raw_p);
        node_alloc_traits::deallocate(m_alloc, p, 1);
        p = nullptr;
    }

    void clearHeadToTail()
    {
        clear();
        if (nullptr != m_header)
        {
            clear(m_header);
        }

        if(nullptr != m_tail)
        {
            clear(m_tail);
        }
    }

    node_pointer backNode() { 
        if (!empty()){
           return m_tail->prev; 
        }
        return m_tail;
    }

    const_node_pointer backNode() const {
        if (!empty()){
           return m_tail->prev; 
        }
        return m_tail;
    }

    node_pointer frontNode() { return m_header->next; }
    node_pointer frontNode() const { return m_header->next; }

    template<typename U>
    void insertAfter(node_pointer p_node, U && val)
    {
        auto p_new_node = create(m_id, p_node, p_node->next, std::forward<U>(val));
        p_node->next->prev = p_new_node;
        p_node->next = p_new_node;
        ++m_size;
    }

    template<typename U>
    void insertBefore(node_pointer p_node, U && val)
    {
//        puts("list: insertBefore()");
        auto p_new_node = create(m_id, p_node->prev, p_node, std::forward<U>(val));
        p_node->prev->next = p_new_node;
        p_node->prev = p_new_node;
        ++m_size;
    }

    node_pointer eraseAt(node_pointer p_node)
    {
        auto p_prev = p_node->prev;
        auto p_next = p_node->next;
        p_prev->next = p_next;
        p_next->prev = p_prev;
        clear(p_node);
        --m_size;
        return p_next;
    }

    void buildSentinelNodes()
    {
        m_header = create(m_id, nullptr, nullptr);
        m_tail = create(m_id, m_header, nullptr);
        m_header->next = m_tail;
    }

    template<bool Const>
    bool validate(node_iterator<Const> pos, [[maybe_unused]] wrap<IteratorOperation::InsertAfter> op)
    {
       auto node_pos = pos.current_node;
       if (nullptr == node_pos || node_pos->id != m_id)
       {
           return false;
       }

       return m_tail != node_pos;
    }

    template<bool Const>
    bool validate(node_iterator<Const> pos, [[maybe_unused]] wrap<IteratorOperation::InsertBefore> op)
    {
       auto node_pos = pos.current_node;
       if (nullptr == node_pos || node_pos->id != m_id)
       {
           return false;
       }

       // should never be true
       return m_header != node_pos;
    }

    template<bool Const>
    bool validate(node_iterator<Const> pos, [[maybe_unused]] wrap<IteratorOperation::EraseAt> op)
    {
        if (end() == pos)
        {
            return false;
        }

        auto node_pos = pos.current_node;
        if (nullptr == node_pos || node_pos->id != m_id)
        {
            return false;
        }

        if (m_header == node_pos || m_tail == node_pos)
        {
            return false;
        }

        auto prev_pos = node_pos->prev;
        auto next_pos = node_pos->next;

        return nullptr != prev_pos && nullptr != next_pos && 
               prev_pos->next == node_pos && next_pos->prev == node_pos;
    }
    
private:
    size_type m_id{ts::getRandomNumberInMinMaxRange<size_type>()};
    size_type m_size{0};
    node_allocator m_alloc{};
    node_pointer m_header{nullptr};
    node_pointer m_tail{nullptr};
};

}//ds
