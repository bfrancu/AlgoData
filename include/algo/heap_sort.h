#pragma once
#include <vector>

namespace algo
{

int left_child(int i)
{
    return 2*i + 1;
}

template<typename T>
void percolate_down(std::vector<T> & arr, int i, int current_size)
{
    auto tmp = std::move(arr[i]);
    int child{0};

    for (; left_child(i) < current_size; i = child)
    {
        child = left_child(i);
        if (child < current_size - 1 && arr[child] < arr[child+1])
        {
            ++child;
        }

        if (tmp < arr[child])
        {
            arr[i] = std::move(arr[child]);
        }
        else
        {
            break;
        }
    }
    arr[i] = std::move(tmp);
}

template<typename T>
void heap_sort(std::vector<T> & arr)
{
    int n = arr.size();
    for (int i = n/2 - 1; i >= 0; --i)
    {
        percolate_down(arr, i, arr.size());
    }

    for (int j = arr.size() - 1; j > 0; --j)
    {
        std::swap(arr[0], arr[j]);
        percolate_down(arr, 0, j);
    }
}

template<typename Iterator, typename Comparator = std::less<decltype(*std::declval<Iterator>())>>
void percolate_down(const Iterator &begin, const Iterator & end, Iterator it, Comparator comp)
{
    int current_dist = std::distance(begin, it);
    Iterator child_it;
    auto tmp = std::move(*it);

    for (; left_child(current_dist) < std::distance(begin, end); current_dist = std::distance(begin, it))
    {
        child_it = begin + left_child(current_dist);

        if (child_it + 1 < end & comp(*child_it, *(child_it+1)))
        {
            ++child_it;
        }

        if (comp(tmp, *child_it))
        {
            *it = std::move(*child_it);
        }
        else
        {
            break;
        }
        it = child_it;
    }
    *it = std::move(tmp);
}

template<typename Iterator, typename Comparator = std::less<decltype(*std::declval<Iterator>())>>
void heap_sort_impl(const Iterator & begin, const Iterator & end, Comparator comp = {})
{
    if (begin == end)
    {
        return;
    }

    auto mid_it = begin + (std::distance(begin, end)/2 - 1);
    for (; mid_it >= begin; --mid_it)
    {
        percolate_down(begin, end, mid_it, comp);
    }

    for (auto it = end - 1; it > begin; --it)
    {
        using namespace std;
        swap(*begin, *it);
        percolate_down(begin, it, begin, comp);
    }
}

}//algo
