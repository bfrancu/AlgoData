#include <numeric>
#include <algorithm>
#include <vector>
#include <iostream>
#include <type_traits>

namespace ts
{

template<typename T>
std::vector<T> generate_sorted_vector(std::size_t size)
{
    std::vector<T> res;
    res.resize(size);

    std::iota(std::begin(res), std::end(res), T{0});
    return res;
}

template<typename T>
std::vector<std::vector<T>> generate_sorted_2d_vector(std::size_t rowSize, std::size_t columnSize)
{
    std::vector<std::vector<T>> res;
    res.resize(rowSize);

    for (auto & arr : res)
    {
        arr = generate_sorted_vector<T>(columnSize);
    }
    return res;
}

template<typename T, std::size_t SIZE>
std::array<T, SIZE> generate_fix_sized_array()
{
    std::array<T, SIZE> res;

    std::iota(std::begin(res), std::end(res), T{0});
    return res;
}

template<typename T, std::size_t ROW_SIZE, std::size_t COL_SIZE = ROW_SIZE>
std::array<std::array<T, COL_SIZE>, ROW_SIZE> generate_fix_sized_2d_array()
{
    std::array<std::array<T, COL_SIZE>, ROW_SIZE> res;
    std::for_each(std::begin(res), std::end(res), [] (auto & arr){
        arr = generate_fix_sized_array<T, COL_SIZE>();
    });
    return res;
}


template<typename T>
void print_vector(std::vector<T> & v)
{
    for (const auto & el : v)
    {
        std::cout << el << " ";
    }
    std::cout << "\n";
}

template<template<typename Printable, typename... Args> typename Collection,
         typename Printable, typename... Args>
void print_collection(const Collection<Printable, Args...> & coll)
{
    std::for_each(std::begin(coll), std::end(coll),
             [] (const auto & el) { std::cout << el << " "; });

    std::cout << "\n";
}

template<template<typename Printable, auto... Vs> typename Collection,
         typename Printable, auto... Vs>
void print_collection(const Collection<Printable, Vs...> & coll)
{
    std::for_each(std::begin(coll), std::end(coll),
             [] (const auto & el) { std::cout << el << " "; });

    std::cout << "\n";
}

template<template<typename Comparable, typename... Args> typename Collection,
         typename Comparable, typename... Args>
bool are_equal(const Collection<Comparable, Args...> & coll1,
               const Collection<Comparable, Args...> & coll2)
{
    if (coll1.size() != coll2.size())
    {
        return false;
    }

    auto it1 = std::begin(coll1);
    auto it2 = std::begin(coll2);
    for (; it1 != std::end(coll1) && it2 != std::end(coll2); ++it1, ++it2)
    {
        if (*it1 != *it2)
        {
            return false;
        }
    }
    return true;
}


template<template<typename... > typename Collection1,
         template<typename... > typename Collection2,
         typename Comparable1, typename Comparable2, typename... Args>
constexpr std::enable_if_t<!std::is_convertible_v<Comparable1, Comparable2>, bool>
are_equal(const Collection1<Comparable1, Args...> & coll1,
          const Collection2<Comparable2, Args...> & coll2)
{
    return false;
}

template<template<typename... > typename Collection1,
         template<typename... > typename Collection2,
         typename Comparable1, typename Comparable2, typename... Args>
constexpr std::enable_if_t<std::is_convertible_v<Comparable1, Comparable2>, bool>
are_equal(const Collection1<Comparable1, Args...> & coll1,
          const Collection2<Comparable2, Args...> & coll2)
{
    return are_equal_impl(coll1, coll2);
}

template<template<typename... > typename Collection,
         typename Comparable1, typename Comparable2, typename... Args>
constexpr std::enable_if_t<std::is_convertible_v<Comparable1, Comparable2>, bool>
are_equal(const Collection<Comparable1, Args...> & coll1,
          const Collection<Comparable2, Args...> & coll2)
{
    return are_equal_impl(coll1, coll2);
}

template<typename Collection1, typename Collection2>
bool are_equal_impl(const Collection1 & coll1, const Collection2 & coll2)
{
    std::cout << "are_equal_impl()\n";
    if (coll1.size() != coll2.size())
    {
        return false;
    }

    auto it1 = std::begin(coll1);
    auto it2 = std::begin(coll2);
    for (; it1 != std::end(coll1) && it2 != std::end(coll2); ++it1, ++it2)
    {
        if (*it1 != *it2)
        {
            return false;
        }
    }
    return true;
}

template<typename Collection1, typename Collection2>
struct equals
{
    static constexpr bool call(...) { return false; }
};

template<template<typename... > typename Collection1,
         typename Collection2, typename Comparable1, typename... Args>
struct equals<Collection1<Comparable1, Args...>, Collection2>
{
    template<typename C2>
    struct helper 
    {
        static constexpr bool call(...) 
        {
            std::cout << "equals::helper<C2>::call()\n";
            return false;
        }
    };

    template<template<typename... > typename C2, typename Comparable2, typename... Xargs>
    struct helper<C2<Comparable2, Xargs...>>
    {
        static constexpr bool call(const Collection1<Comparable1, Args...> & c1, 
                                   const C2<Comparable2, Xargs...> & c2)
        {
            std::cout << "equals::helper<C2, Comparable, Xargs>::call()\n";
            if constexpr (std::is_convertible_v<Comparable1, Comparable2>)
            {
                return are_equal_impl(c1, c2);
            }
            else 
            {
                return false;
            }
        }
    };

    template<template<typename, auto...> typename C2, typename Comparable2, auto... Vs>
    struct helper<C2<Comparable2, Vs...>>
    {
        static constexpr bool call(const Collection1<Comparable1, Args...> & c1, 
                                   const C2<Comparable2, Vs...> & c2)
        {
            std::cout << "equals::helper<C2, Comparable, Vs>::call()\n";
            if constexpr (std::is_convertible_v<Comparable1, Comparable2>)
            {
                return are_equal_impl(c1, c2);
            }
            else 
            {
                return false;
            }
        }
    };

    static bool call(const Collection1<Comparable1, Args...> & c1, const Collection2 & c2)
    {
        std::cout << "equals::call()\n";
        return helper<Collection2>::call(c1, c2);
    }
};

template<typename C1, typename C2>
bool equivalent(const C1& c1, const C2& c2)
{
    std::cout << "equivalent()\n";
    return equals<C1, C2>::call(c1, c2);
}

}//ts
