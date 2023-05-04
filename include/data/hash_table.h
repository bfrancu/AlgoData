#pragma once
#include <functional>
#include <type_traits>
//#include <vector>
#include "vector.h"
#include "tools.h"


namespace ds
{

template<typename Key,
         typename Hash = std::hash<Key>,
         typename KeyEqual = std::equal_to<Key>>
class unordered_set
{
    static_assert(!ts::is_reference_v<Key>, "unordered_set error: type reference not allowed");
    template<typename... Args>
    using Container = ds::vector<Args...>;
    using Bucket = Container<Key>;
    using BucketIterator = typename Bucket::iterator;
    using BucketConstIterator = typename Bucket::const_iterator;
    using BucketContainer = Container<Bucket>;

public:
    explicit unordered_set(std::size_t size = 101)
    {
        m_buckets.resize(101);
    }

public:
    bool contains(const Key & value) const
    {
        auto index{generateHash(value)};
        auto & bucket{m_buckets[index]};
        auto [res, it] = contains(bucket, value, m_equal);
        return res;
    }

    template<typename K>
    bool insert(K && value)
    {
        auto index{generateHash(value)};
//        std::cout << "unordered_set::insert() value: " << value << " generated index: " << index << "\n";
        auto & bucket{m_buckets[index]};

        if (auto [res, it] = contains(bucket, value, m_equal); !res)
        {
            bucket.push_back(std::forward<K>(value));
            bucket.push_back(std::forward<K>(value));
            ++m_currentSize;

            if (m_currentSize >= m_buckets.size())
            {
                rehash();
            }

            return true;
        }

        return false;
    }

    bool remove(const Key & value)
    {
        auto index{generateHash(value)};
        auto & bucket{m_buckets[index]};

        if (auto [res, it] = contains(bucket, value, m_equal); res)
        {
            bucket.erase(it);
            --m_currentSize;
            return true;
        }

        return false;
    }

    void clear()
    {
        for (auto & bucket : m_buckets)
        {
            bucket.clear();
        }
        m_currentSize = 0;
    }

    
private:
    static __attribute__((always_inline)) std::pair<bool, BucketConstIterator> 
    contains(const Bucket & bucket, const Key & value, const KeyEqual & equal)
    {
        auto it = std::find_if(std::begin(bucket), std::end(bucket), [&equal, &v = std::as_const(value)] (const auto & item){ 
                               return equal(v, item); });

        return std::make_pair(std::end(bucket) != it, it);
    }

    std::size_t generateHash(const Key & value) const
    {
//        std::cout << "unordered_set::generateHash() value: " << value 
//                  << " hash(value): " << m_hash(value) 
//                  << " index: " << m_hash(value) % m_buckets.size() << "\n";
        return m_hash(value) % m_buckets.size();
    }

    void rehash()
    {
        auto newSize = ts::nextPrime(m_buckets.size()*2);
        std::cout << "unordered_set::rehash() new size: " << newSize << "\n";
        BucketContainer oldTable{};
        transferElements(m_buckets, oldTable);
        clear();
        m_buckets.resize(newSize);
        for (auto & bucket : oldTable)
        {
            for (auto & el : bucket)
            {
                if constexpr(std::is_move_constructible_v<Key>)
                {
                    insert(std::move(el));
                }
                else
                {
                    insert(el);
                }
            }
        }
    }

    void transferElements(BucketContainer & in, BucketContainer & out)
    {
        out.reserve(in.size());
        for (auto & srcBucket : in)
        {
            out.push_back({});
            auto & destBucket{out.back()}; 
            destBucket.reserve(srcBucket.size());
            if constexpr(std::is_move_constructible_v<Key>)
            {
                std::move(std::begin(srcBucket), std::end(srcBucket), std::back_inserter(destBucket));
            }
            else
            {
                std::copy(std::begin(srcBucket), std::end(srcBucket), std::back_inserter(destBucket));
            }
        }
    }

private:
    std::size_t m_currentSize{0};
    Hash m_hash{};
    KeyEqual m_equal{};
    BucketContainer m_buckets;
};

}//ds
