#pragma once
#include <stdexcept>
#include <utility>

#include "iterator_facade.h"
#include "traits.h"
#include "tools.h"
#include "base_member_pair.hpp"

namespace ds
{

template<typename T,
         typename Allocator = std::allocator<T>>
class vector
{
    static_assert(!ts::is_reference_v<T>, "vector error: type reference not allowed");

    template<bool>
    class internal_iterator;

    using alloc_traits = std::allocator_traits<Allocator>;

public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename alloc_traits::pointer;
    using const_pointer = typename alloc_traits::const_pointer;
    using iterator = internal_iterator<false>;
    using const_iterator = internal_iterator<true>;

private:
    template<bool Const>
    class internal_iterator : public iterator_facade<internal_iterator<Const>,
                                                     std::conditional_t<Const, T, const T>,
                                                     std::bidirectional_iterator_tag>
    {

        using base = iterator_facade<internal_iterator<Const>, std::conditional_t<Const, T, const T>, std::bidirectional_iterator_tag>;

        friend iterator_facade<internal_iterator<Const>, std::conditional_t<Const, T, const T>, std::bidirectional_iterator_tag>;
        friend vector;

    public:
        using value_type = typename base::value_type;
        using reference = typename base::reference;
        using pointer = typename vector::pointer;
        using difference_type = typename vector::difference_type;
        using iterator_category = typename base::iterator_category;

    protected:
        internal_iterator(pointer p) :
            p_value{p}
        {}
        
        template<bool C>
        bool equals(const internal_iterator<C> & other) const
        {
            return p_value == other.p_value;
        }

        __attribute__((always_inline)) auto & dereference() const
        {
            return *p_value;
        }

        void increment()
        {
            ++p_value;
        }
        
        void decrement()
        {
            --p_value;
        }

        void advance(difference_type n)
        {
            p_value += n;
        }

        template<bool C>
        difference_type measureDistance(const internal_iterator<C> & other) const
        {
            return std::distance(item(), other.item());
        }

    protected:
        pointer item() const { return p_value; }

    private:
        pointer p_value;
    };


public:
    vector(const Allocator & alloc = Allocator()) :
        m_compressedPair(alloc, 0)
    {}

    vector(const vector & other) :
        m_compressedPair(other.getAlloc(), 0)
    {
        try
        {
            insert(end(), other.begin(), other.end());
        }
        catch(...)
        {
            clear();
        }
    }

    vector(vector && other) :
        m_compressedPair(other.getAlloc(), std::exchange(other.getSize(), 0))
    {
        m_pArray = std::exchange(other.m_pArray, nullptr);
        m_capacity = std::exchange(other.m_capacity, 0);
    }

    vector & operator=(const vector & other)
    {
        clear();
        constexpr bool pocca{std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value};
        if constexpr (pocca){
            getAlloc() = other.getAlloc();
        }

        insert(end(), other.begin(), other.end());
        return *this;
    }

    vector & operator=(vector && other)
    {
       clear();
       constexpr bool pocma{std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value};
       if constexpr (pocma){
           getAlloc() = other.getAlloc();
           getSize() = std::exchange(other.getSize(), 0);
           m_pArray = std::exchange(other.m_pArray, nullptr);
           m_capacity = std::exchange(other.m_capacity, 0);
       }
       else if (getAlloc() == other.getAlloc()){
           getSize() = std::exchange(other.getSize(), 0);
           m_pArray = std::exchange(other.m_pArray, nullptr);
           m_capacity = std::exchange(other.m_capacity, 0);
       }
       else{
           try
           {
               insert(end(), other.begin(), other.end());
           }
           catch(...)
           {
               clear();
           }
           other.clear();
       }
       return *this;
    }

    void swap(vector & other)
    {
        constexpr bool poccs{std::allocator_traits<allocator_type>::propagate_on_container_swap::value};
        using std::swap;

        swap(m_capacity, other.m_capacity);
        swap(getSize(), other.getSize());

        if constexpr (poccs){
            swap(getAlloc(), other.getAlloc());
            swap(m_pArray, other.m_pArray);
        }
        else if (getAlloc() == other.getAlloc()){
            swap(m_pArray, other.m_pArray);
        }
        else{
            auto temp = std::move(*this);
            *this = std::move(other);
            other = std::move(temp);
        }
    }

    explicit vector(size_type count, const T & value, const Allocator & alloc = Allocator()) :
        m_compressedPair(alloc, 0)
    {
        try
        {
            initArray(count, value);
        }
        catch(...)
        {
            clear();
        }
    }

    explicit vector(size_type count, const Allocator & alloc = Allocator()) :
       m_compressedPair(alloc, 0)
    {
        try
        {
            initArray(count);
        }
        catch(...)
        {
            clear();
        }
    }

    template<typename InputIt>
    vector(InputIt first, InputIt last, const Allocator & alloc = Allocator()) :
       m_compressedPair(alloc, 0)
    {
        try
        {
            prepareStorageCapacity(std::distance(first, last));
            insert(end(), first, last);
        }
        catch(...)
        {
            clear();
        }
    }

    vector(std::initializer_list<T> ilist, const Allocator & alloc = Allocator()) :
       m_compressedPair(alloc, 0)
    {
        try
        {
            prepareStorageCapacity(ilist.size());
            insert(end(), ilist);
        }
        catch(...)
        {
            clear();
        }
    }

    void push_back(const T& value)
    {
        prepareStorageCapacity();
        auto & size{getSize()};
        T *raw_p = std::addressof(m_pArray[size]);
        alloc_traits::construct(getAlloc(), raw_p, value);
        ++size;
    }

    void push_back(T&& value)
    {
        prepareStorageCapacity();
        auto & size{getSize()};
        T *raw_p = std::addressof(m_pArray[size]);
        alloc_traits::construct(getAlloc(), raw_p, std::move(value));
        ++size;
    }

    void pop_back()
    {
        auto & size{getSize()};
        if (likely(0 != size))
        {
            T *raw_p = std::addressof(m_pArray[size-1]);
            alloc_traits::destroy(getAlloc(), raw_p);
            --size;
        }
    }

    void clear()
    {
        clear(m_pArray, m_capacity);
        getSize() = 0;
    }

    void reserve(size_type capacity)
    {
        if (capacity > m_capacity)
        {
            auto newCapacity = ts::nextLargerPowerOf2(capacity);
            increaseStorage(newCapacity);
        }
    }

    void resize(size_type count)
    {
        auto size{getSize()};
        if (count > size){
            expandToSize(count);
        }
        else if (count < size){
            shrinkToSize(count);
        }
    }

    template<bool C>
    iterator erase(internal_iterator<C> pos)
    {
        return iterator(erase(pos.item()));
    }

    template<bool C>
    iterator erase(internal_iterator<C> first, internal_iterator<C> last)
    {
        return iterator(erase(first.item(), last.item()));
    }

    template<bool C>
    iterator insert(internal_iterator<C> pos, T && value)
    {
        return iterator(insert(pos.item(), std::move(value)));
    }

    template<bool C>
    iterator insert(internal_iterator<C> pos, const T & value)
    {
        return iterator(insert(pos.item(), value));
    }

    template<bool C, typename InputIt>
    iterator insert(internal_iterator<C> pos, InputIt first, InputIt last)
    {
        return iterator(insert(pos.item(), first, last));
    }

    template<bool C>
    iterator insert(internal_iterator<C> pos, std::initializer_list<T> ilist)
    {
        return insert(pos.item(), ilist.begin(), ilist.end());
    }

    iterator begin() { return iterator(startPtr()); }
    const_iterator begin() const { return const_iterator(startPtr()); }
    const_iterator cbegin() const noexcept { return begin(); }

    iterator end() { return iterator(endPtr()); }
    const_iterator end() const { return const_iterator(endPtr()); }
    const_iterator cend() const noexcept { return end(); }

    reference front() { return *(startPtr()); }
    const_reference front() const { return *(startPtr()); }
    reference back() { return *(backPtr()); }
    const_reference back() const { return *(backPtr()); }

    reference at(size_type pos) {
        if (likely(pos < getSize())){
            return operator[](pos);
        }
        throw std::out_of_range{"index out of bounds"};
    }

    const_reference at(size_type pos) const {
        if (likely(pos < getSize())){
            return operator[](pos);
        }
        throw std::out_of_range{"index out of bounds"};
    }

    reference operator[](size_type pos){ return *(m_pArray + pos); }
    const_reference operator[](size_type pos) const{ return *(m_pArray + pos); }

    bool empty() const { return 0 == getSize(); }
    size_type size() const { return getSize(); }
    size_type capacity() const { return m_capacity; }

private:
    template<typename... Args>
    void initArray(size_type count, Args&&... args)
    {
        m_pArray = create(count, std::forward<Args>(args)...);
        getSize() = count;
        m_capacity = count;
    }

    template<typename... Args>
    pointer create(size_type count, Args&&... args)
    {
        auto p_array = allocate(count);
        construct(p_array, count, std::forward<Args>(args)...);
        return p_array;
    }

    pointer allocate(size_type count) { return alloc_traits::allocate(getAlloc(), count); }

    template<typename... Args>
    void construct(pointer p, size_type count, Args&&... args)
    {
        auto & alloc = getAlloc();
        for (size_type i = 0; i < count; ++i) {
            try
            {
                T *raw_p = static_cast<T*>(std::addressof(p[i]));
                alloc_traits::construct(alloc, raw_p, std::forward<Args>(args)...);
            }

            catch(...)
            {
                break;
                for (size_type j = 0; j < i; ++j) {
                    T *raw_p = static_cast<T*>(std::addressof(p[j]));
                    alloc_traits::destroy(alloc, raw_p);
                }
                alloc_traits::deallocate(alloc, p, count);
                p = nullptr;
                throw;
            }
        }
    }

    /*
    template<typename Alloc,
             typename std::enable_if_t<!std::is_move_constructible_v<typename std::allocator_traits<Alloc>::value_type>, int> = 0, 
             typename = void>
    static inline void constructElement(Alloc & alloc, typename std::allocator_traits<Alloc>::pointer p_src,
                                        typename std::allocator_traits<Alloc>::pointer p_dest, size_type srcIdx, size_type destIdx)
    {
        T *raw_p = static_cast<T*>(std::addressof(p_dest[destIdx]));
        alloc_traits::construct(alloc, raw_p, p_src[srcIdx]);
    }

    template<typename Alloc,
             typename std::enable_if_t<std::is_move_constructible_v<typename std::allocator_traits<Alloc>::value_type>, bool> = true>
    static inline void constructElement(Alloc & alloc, typename std::allocator_traits<Alloc>::pointer p_src,
                                        typename std::allocator_traits<Alloc>::pointer p_dest, size_type srcIdx, size_type destIdx)
    {
        T *raw_p = static_cast<T*>(std::addressof(p_dest[destIdx]));
        alloc_traits::construct(alloc, raw_p, std::move(p_src[srcIdx]));
    }
    
    __attribute__((always_inline))
    static void constructElement(Allocator & alloc, pointer p_dest, size_type destIdx)
    {
        T *raw_p = static_cast<T*>(std::addressof(p_dest[destIdx]));
        alloc_traits::construct(alloc, raw_p);
    }
    */

    bool constructFrom(pointer p_old, pointer p_new, size_type size, size_type capacity,
                       size_type startGapIdx = 0, size_type gapLength = 0)
    {
        if (unlikely(capacity < size && (startGapIdx + gapLength > capacity || startGapIdx > size)))
        {
            return false;
        }

        auto & alloc = getAlloc();
        size_type i{0};
        size_type j{0};
        size_type endGapIdx{startGapIdx + gapLength};

        try{
            for(; i < startGapIdx; ++i)
            {
                //constructElement(alloc, p_old, p_new, i, i);
                T *raw_p = static_cast<T*>(std::addressof(p_new[i]));
                if constexpr (std::is_move_constructible_v<value_type>)
                {
                    alloc_traits::construct(alloc, raw_p, std::move(p_old[i]));
                }
                else
                {
                    alloc_traits::construct(alloc, raw_p, p_old[i]);
                }
            }

            j = i;
            for (; j < endGapIdx; ++j) {}
            
            j = endGapIdx;
            for (; i < size && j < capacity; ++i)
            {
//                constructElement(alloc, p_old, p_new, i, j++);
                T *raw_p = static_cast<T*>(std::addressof(p_new[j]));
                if constexpr (std::is_move_constructible_v<value_type>)
                {
                    alloc_traits::construct(alloc, raw_p, std::move(p_old[i]));
                }
                else
                {
                    alloc_traits::construct(alloc, raw_p, p_old[i]);
                }
                ++j;
            }
        }
        catch(...) {
           for(size_type k = 0; k < startGapIdx; ++k){
               T *raw_p = static_cast<T*>(std::addressof(p_new[k]));
               alloc_traits::destroy(alloc, raw_p);
           }

           for (size_type k = endGapIdx; k < j; ++k){
               T *raw_p = static_cast<T*>(std::addressof(p_new[k]));
               alloc_traits::destroy(alloc, raw_p);
           }

           alloc_traits::deallocate(alloc, p_new, capacity);
           p_new = nullptr;
           throw;
        }
        return true;
    }

    template<typename InputIt>
    pointer assignFrom(pointer p_where, InputIt first, InputIt last)
    {
        auto p_start = p_where;
        for (auto it = first; it != last;  ++it) {
            *p_where = *it;
            ++p_where;
        }
        return p_start;
    }

    template<typename InputIt>
    pointer constructHere(pointer p_where, InputIt first, InputIt last)
    {
        auto p_start = p_where;
        auto & alloc = getAlloc();
        for (auto it = first; it != last;  ++it)
        {
            T *raw_p = static_cast<T*>(p_where);
            alloc_traits::construct(alloc, raw_p, *it);
            ++p_where;
        }
        return p_start;
    }

    //        |<- start gap
    //1 3 4 5 8 5 7 9 3 4 0 0 0 0 0
    //        |<----->| gap size
    //1 3 4 5 0 0 0 0 8 5 7 9 3 4 0
    //1 3 4 5 8 5 7 9 3 0 0 0 0 4 0
    bool makeGapInCurrentStorage(size_type startGapIdx, difference_type gapLength)
    {
        auto size = getSize();
        if (unlikely(size + gapLength > m_capacity))
        {
            return false;
        }

        auto & alloc = getAlloc();
        auto end{startGapIdx > 0 ? startGapIdx : 1};
        for (size_type i = size - 1; i >= end; --i)
        {
              T *raw_p = std::addressof(m_pArray[i+gapLength]);
              alloc_traits::construct(alloc, raw_p, m_pArray[i]);
              raw_p = std::addressof(m_pArray[i]);
              alloc_traits::destroy(alloc, raw_p);
        }
        return true;
    }

    //      start gap->|                 |<- end gap            size->|         |<-capacity
    //1 9 2 4 5 2 35 7 0  0  0  0  0  0  0 33 21 26 31 43 12 5 7 92 61 87 0 0 0 0 
    //1 9 2 4 5 2 35 7 33 21 26 31 43 12 5 0 0 0 0 0 0 0 7 92 61 87 0 0 0 0 
    //1 9 2 4 5 2 35 7 33 21 26 31 43 12 5 7 0 0 0 0 0 0 0 92 61 87 0 0 0 0 
    //1 9 2 4 5 2 35 7 33 21 26 31 43 12 5 7 92 0 0 0 0 0 0 0 61 87 0 0 0 0 
    //1 9 2 4 5 2 35 7 33 21 26 31 43 12 5 7 92 61 87 0 0 0 0 0 0 0 0 0 0 0
    bool closeGapInCurrentStorage(size_type startGapIdx, difference_type gapLength)
    {
        auto size = getSize();
        if(unlikely(startGapIdx + gapLength > size - gapLength))
        {
            return false;
        }

        using std::swap;
        for (size_type i = startGapIdx; i < size - 1; ++i)
        {
            swap(m_pArray[i], m_pArray[i+gapLength]);
        }
        return true;
    }
    
    bool increaseStorage(size_type storageCapacity, difference_type startGapIdx = 0, size_type gapLength = 0)
    {
        if (unlikely(m_capacity > storageCapacity))
        {
            return false;
        }

        auto p_newStorage = allocate(storageCapacity);
        if (constructFrom(m_pArray, p_newStorage, getSize(), storageCapacity, startGapIdx, gapLength))
        {
            auto currentSize{getSize()};
            clear();
            getSize() = currentSize;
            m_pArray = p_newStorage;
            m_capacity = storageCapacity;
            return true;
        }

        return false;
    }

    void clear(pointer p, size_type count)
    {
        if (unlikely(nullptr == p))
        {
            return;
        }

        auto & alloc = getAlloc();
        for (size_type i = 0; i < count; ++i)
        {
            T *raw_p = static_cast<T*>(std::addressof(p[count]));
            alloc_traits::destroy(alloc, raw_p);
        }

        alloc_traits::deallocate(alloc, p, count);
        p = nullptr;
    }

    bool preparePositionalInsert(difference_type insertionIndex, size_type inputSize)
    {
        auto size = getSize();
        if (inputSize < m_capacity - size && 0 != size)
        {
            return makeGapInCurrentStorage(insertionIndex, inputSize);
        }

        return prepareStorageCapacity(inputSize, insertionIndex);
    }

    bool prepareStorageCapacity(size_type inputSize = 0, difference_type insertionIndex = 0) 
    {
        auto size{getSize()};
        if (size >= m_capacity)
        {
            auto storageCapacity{m_capacity > inputSize ? m_capacity*2 : ts::nextLargerPowerOf2(inputSize + size + 1)};
            if (likely(0 != m_capacity))
            {
                return increaseStorage(storageCapacity, insertionIndex, inputSize);
            }
            m_pArray = allocate(storageCapacity);
            m_capacity = storageCapacity;
        }
        return true;
    }

    void shrinkToSize(size_type newSize)
    {
        auto & size{getSize()};
        auto & alloc{getAlloc()};
        if (newSize < size)
        {
            for (size_type i = size - 1; i > newSize - 1; --i)
            {
                T *raw_p = std::addressof(m_pArray[i]);
                alloc_traits::destroy(alloc, raw_p);
            }
            size = newSize;
        }
    }

    void expandToSize(size_type newSize)
    {
        auto & size{getSize()};
        auto sizeNeeded{newSize - size};
        if (prepareStorageCapacity(sizeNeeded))
        {
            construct(m_pArray[size], sizeNeeded);
        }
    }

    bool validate(pointer p_item)
    {
        if (nullptr == p_item)
        {
            return false;
        }

        auto res{ts::belongsToRange(p_item, m_pArray, m_capacity)};
        return res;
    }

    void checkForNullStorage(pointer & p_item,  size_type inputSize = 1)
    {
        if (emptyStorage())
        {
            prepareStorageCapacity(inputSize);
            if (nullptr == p_item){
                p_item = endPtr();
            }
        }
    }

    pointer erase(pointer first, pointer last)
    {
        if (!(validate() && validate(last))){
            return endPtr();
        }

        auto erasureSize{std::distance(first, last)};
        auto erasureIndex{std::distance(m_pArray, first)};
        if (closeGapInCurrentStorage(erasureIndex, erasureSize))
        {
            auto & size{getSize()};
            size -= erasureSize;
            return std::addressof(m_pArray[erasureIndex]);
        }
        return endPtr();
    }

    pointer erase(pointer where)
    {
        if (!validate(where)) {
            return endPtr();
        }

        auto erasureIndex{std::distance(m_pArray, where)};
        if (closeGapInCurrentStorage(erasureIndex, 1))
        {
            auto & size{getSize()};
            --size;
            return std::addressof(m_pArray[erasureIndex]);
        }
        return endPtr();
    }

    template<typename U>
    pointer insert(pointer where, U && value)
    {
        if (checkForNullStorage(where); !validate(where))
        {
            return endPtr();
        }

        auto insertionIndex{std::distance(m_pArray, where)};
        if (preparePositionalInsert(insertionIndex, 1))
        {
            auto & size{getSize()};
            T *raw_p = std::addressof(m_pArray[insertionIndex]);
            alloc_traits::construct(getAlloc(), raw_p, std::forward<U>(value));
            ++size;
            return std::addressof(m_pArray[insertionIndex]);
        }

        return endPtr();
    }

    template<typename InputIt>
    pointer insert(pointer where, InputIt first, InputIt last)
    {
        auto inputSize{std::distance(first, last)};
        auto & size{getSize()};
        if (checkForNullStorage(where, inputSize); !validate(where))
        {
            return endPtr();
        }

        auto insertionIndex{std::distance(m_pArray, where)};
        if (preparePositionalInsert(insertionIndex, inputSize))
        {
            auto p_insertHere = std::addressof(m_pArray[insertionIndex]);
            size += inputSize;
            return constructHere(p_insertHere, first, last);
        }

        return endPtr();
    }

    pointer endPtr() const { return m_pArray + getSize(); }
    pointer backPtr() const { return m_pArray + getSize() - 1; }
    pointer startPtr() const { return m_pArray; }

    size_type & getSize() { return m_compressedPair.member(); }
    const size_type & getSize() const { return m_compressedPair.member(); }
    Allocator & getAlloc() { return m_compressedPair.base(); }
    const Allocator & getAlloc() const { return m_compressedPair.base(); }

    bool emptyStorage() const { return 0 == m_capacity; }

private:
    size_type m_capacity{0};
    pointer m_pArray{nullptr};
    ts::BaseMemberPair<Allocator, size_type> m_compressedPair{};
};
}//ds
