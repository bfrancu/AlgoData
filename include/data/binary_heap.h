#pragma once
#include <optional>
#include <functional>
#include <algorithm>
#include <vector>

template<typename Comparable, typename Comparator = std::less<Comparable>>
class binary_heap
{
public:
    explicit binary_heap(std::size_t capacity = 100);
    explicit binary_heap(const std::vector<Comparable> & items);

public:
    bool is_empty() const { return 0 == m_current_size; }
    std::size_t size() const { return m_current_size; }
    std::optional<std::reference_wrapper<const Comparable>> first() const;

    template<typename C>
    void insert(C && item);

    void pop();
    void pop(Comparable & item);
    void clear();

    template<typename UnaryF>
    void for_each(UnaryF f);

protected:
    void build_heap();
    void percolate_down(std::size_t hole);

private:
    std::size_t m_current_size{};
    std::vector<Comparable> m_array{};
    Comparator m_comp{};

};

template<typename Comparable, typename Comparator>
binary_heap<Comparable, Comparator>::binary_heap(std::size_t capacity) 
{
    if (capacity < m_array.max_size())
    {
        m_array.reserve(capacity);
    }
}

template<typename Comparable, typename Comparator>
binary_heap<Comparable, Comparator>::binary_heap(const std::vector<Comparable> & items)
{
    m_array.emplace_back(Comparable());
    std::copy(std::begin(items), std::end(items), std::back_inserter(m_array));
    build_heap();
}

template<typename Comparable, typename Comparator> template<typename C>
void binary_heap<Comparable, Comparator>::insert(C && item)
{
    if (m_current_size == m_array.size() - 1)
    {
        m_array.resize(m_current_size*2 + 1);
    }

    m_array[0] = std::forward<C>(item);

    std::size_t hole{++m_current_size};
    auto parentOf = [] (std::size_t index) { return index/2; };

    for (; hole > 0 && m_comp(m_array[0], m_array[parentOf(hole)]); hole = parentOf(hole))
    {
        std::cout << "inserting " << m_array[parentOf(hole)] << " in place of " << m_array[hole] << "\n";
        m_array[hole] = std::move(m_array[parentOf(hole)]);
    }

    m_array[hole] = std::move(m_array[0]);
    std::cout << "binary_heap::insert() hole: " << hole << "; current_size: " 
              << m_current_size << "; value " << m_array[hole] << "\n";
}

template<typename Comparable, typename Comparator> template<typename UnaryF>
void binary_heap<Comparable, Comparator>::for_each(UnaryF f)
{
    std::for_each(std::begin(m_array) + 1, std::begin(m_array) + m_current_size + 1, f);
}

template<typename Comparable, typename Comparator> 
void binary_heap<Comparable, Comparator>::percolate_down(std::size_t hole)
{
    int child{0};
    auto tmp{std::move(m_array[hole])};

    std::cout << "binary_heap::percolate_down() finding a place for " << tmp << "\n";
    for_each([] (auto val) { std::cout << val << ", " ; });
    std::cout << "\n";

    for(; hole*2 <= m_current_size; hole = child)
    {
        child = hole*2;
        if (child != m_current_size && m_comp(m_array[child+1], m_array[child]))
        {
            ++child;
        }
        if (m_comp(m_array[child], tmp))
        {
            std::cout << "binary_heap::percolate_down() inserting " << m_array[child] 
                      << " in place of " << m_array[hole] << "\n";
            m_array[hole] = std::move(m_array[child]);
            for_each([] (auto val) { std::cout << val << ", " ; });
            std::cout << "\n";
        }
        else
        {
            break;
        }
    }
    m_array[hole] = std::move(tmp);
    std::cout <<"binary_heap::percolate_down() end of percolation\n";
    for_each([] (auto val) { std::cout << val << ", " ; });
    std::cout << "\n";
}

template<typename Comparable, typename Comparator>
void binary_heap<Comparable, Comparator>::build_heap()
{
    m_current_size = m_array.size();
    m_array.resize(m_current_size*2 + 1);

    for (int i = 1; i < m_current_size/2 + 1; ++i)
    {
        percolate_down(i);
    }
}

template<typename Comparable, typename Comparator>
void binary_heap<Comparable, Comparator>::pop()
{
    if (is_empty())
    {
        return;
    }

    std::cout << "binary_heap::pop() removing " << m_array[1] << "\n";
    m_array[1] = std::move(m_array[m_current_size--]);
    percolate_down(1);
}

template<typename Comparable, typename Comparator>
void binary_heap<Comparable, Comparator>::pop(Comparable & item)
{
    if (is_empty())
    {
        return;
    }

    item = m_array[1];
    pop();
}

template<typename Comparable, typename Comparator>
void binary_heap<Comparable, Comparator>::clear()
{
    m_array.clear();
    m_current_size = 0;
}

template<typename Comparable, typename Comparator>
std::optional<std::reference_wrapper<const Comparable>> binary_heap<Comparable, Comparator>::first() const
{
    if (is_empty())
    {
        return {};
    }
    return std::optional(std::cref(m_array[1]));
}
