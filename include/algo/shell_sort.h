#pragma once

#include <vector>

namespace algo
{

template<typename T>
void shell_sort(std::vector<T> & arr)
{
    for (int gap = arr.size()/2; gap > 0; gap /= 2)
    {
        for (int p = gap; p < arr.size(); ++p)
        {
            int j = p;

            auto tmp = std::move(arr[j]);
            for (; j >= gap && arr[j] < arr[j-gap]; j -= gap)
            {
                arr[j] = std::move(arr[j-gap]);
            }
            arr[j] = std::move(tmp);
        }
    }
}

template<typename Iterator, typename Comparator = std::less<decltype(*std::declval<Iterator>())>>
void shell_sort_impl(const Iterator & begin, const Iterator & end, Comparator comp = {})
{
    if (begin == end)
    {
        return;
    }

    auto gap = std::distance(begin, end) / 2;
    for (; gap > 0; gap /= 2)
    {
        for (auto p = begin + gap; p != end; ++p)
        {
            auto j = p;
            auto tmp = std::move(*j);
            for (; j >= begin + gap && comp(tmp, *(j-gap)); j -= gap)
            {
                *j = std::move(*(j-gap));
            }
            *j = std::move(tmp);
        }
    }
}

}//algo
