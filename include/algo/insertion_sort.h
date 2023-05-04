#pragma once
#include <vector>

namespace algo
{

template<typename T>
void insertion_sort(std::vector<T> & arr)
{
    for (int p = 1; p < arr.size(); ++p)
    {
        int j = p;
        auto tmp = std::move(arr[j]);

        for (; j > 0 && tmp < arr[j-1]; --j)
        {
            arr[j] = std::move(arr[j-1]);
        }

        arr[j] = std::move(tmp);
    }
}


template<typename Iterator, typename Comparator = std::less<decltype(*std::declval<Iterator>())>>
void insertion_sort_impl(const Iterator & begin, const Iterator & end, Comparator comp = {})
{
    if (begin == end)
    {
        return;
    }

    for (auto p = begin + 1; p != end; ++p)
    {
        auto tmp = std::move(*p);
        auto j = p;
        for (; j > begin && comp(tmp, *(j-1)); --j)
        {
            *j = std::move(*(j-1));
        }
        *j = std::move(tmp);
    }
}

}//algo
