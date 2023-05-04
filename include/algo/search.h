#pragma once
#include <vector>
#include "collection_tools.hxx"

namespace algo
{

static constexpr std::size_t CACHELINE_LEN{64};

template<typename T, std::size_t N>
bool contains(T (&arr)[N], const T & value)
{
    return false;
}

template<template<typename... > typename Coll>
std::pair<bool, size_t> branchyBinarySearch(const Coll<int> & arr, int value)
{
    size_t hi{std::size(arr)-1};
    size_t lo{0};
    while (hi >= lo)
    {
        size_t mid{(lo+hi)/2};
        if (arr[mid] > value)
        {
            hi = mid - 1;
        }
        else if (arr[mid] < value)
        {
            lo = mid + 1;
        }
        else
        {
            return std::make_pair(true, mid);
        }
    }
    return std::make_pair(false, 0);
}

template<template<typename... > typename Coll>
std::pair<bool, size_t> branchFreeBinarySearch(const Coll<int> & arr, int value)
{
    std::cout << "branchFreeBinarySearch() searching for " << value << "\n";
    auto *data = std::data(arr);
    auto *base = data;
    std::size_t n = std::size(arr);

    while(n > 1){
        std::size_t half = n / 2;
        std::cout << "base is now at " << *base 
                  << "; base half: " << base[half] << "; n = " << n << "\n";
        base = (base[half] < value) ? std::addressof(base[half]) : base;
        n -= half;
    }
    
    auto resIdx{(*base < value) + base - data};
    return std::make_pair(value == data[resIdx], resIdx);
}

//https://algorithmica.org/en/eytzinger
template<template<typename... > typename Coll>
std::pair<bool, size_t> eytzingerSearch(const Coll<int> & arr, int value)
{
    int k{1};
    auto sz{std::size(arr)};
    auto base{std::data(arr)};
    while (k <= sz)
    {
        // this is not a simple access pattern; it helps explicitly telling the compiler the prefetch location
        __builtin_prefetch(base + k * CACHELINE_LEN/sizeof(int));
        k = 2 * k + (arr[k] < value);
    }
    // get the trailing 1s in the binary notation of k
    // they represent the number of right turns (2*k+1)
    k >>= __builtin_ffs(~k);
    return std::make_pair(value == arr[k], k);
}


template<template<typename... > typename Coll>
int eytzinger(const Coll<int> & src, Coll<int> & dst, int i = 0, int k = 1)
{
    auto dstSize{std::size(dst)};
    if (dstSize < std::size(src) || -1 == i)
    {
//        std::cout << "eytzinger() dstSize " << dstSize << " < srcSize " << std::size(src) << "\n";
        return -1;
    }

    if (k <= std::size(src))
    {
        i = eytzinger(src, dst, i, 2*k);
//        std::cout << "eytzinger() setting dst[" << k << "] = src[" << i << "] " << src[i] << "\n";
        dst[k] = src[i++];
//        ts::print_vector(dst);
        i = eytzinger(src, dst, i, 2*k+1);
    }
    else
    {
//        std::cout << "eytzinger() k " << k << " bigger than src size " << std::size(src) << "\n";
    }
    return i;
}

}//algo
