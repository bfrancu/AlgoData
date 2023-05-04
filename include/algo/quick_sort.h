#pragma once

#include <vector>

#include "insertion_sort.h"

namespace algo
{

template<typename T>
const T & median3(std::vector<T> & arr, int left, int right)
{
    using namespace std;
    int middle = (left + right) /2;
    if (arr[middle] < arr[left])
    {
        swap(arr[middle], arr[left]);
    }
    if (arr[right] < arr[left])
    {
        swap(arr[left], arr[right]);
    }
    if (arr[right] < arr[middle])
    {
        swap(arr[right], arr[middle]);
    }

    swap(arr[middle], arr[right-1]);
    return arr[right-1];
}

template<typename T>
void quick_sort(std::vector<T> & arr, int left, int right)
{
    if (right - left > 10)
    {
        using namespace std;
        const auto & pivot = median3(arr, left, right);

        int i = left;
        int j = right - 1;
        for (;;)
        {
            if (arr[++i] < pivot) {}
            if (pivot < arr[--j]) {}
            if (i < j)
            {
                swap(arr[i], arr[j]);
            }
            else
            {
                break;
            }
        }

        swap(arr[i], arr[right-1]);
        quick_sort(arr, left, i-1);
        quick_sort(arr, i, right);
    }
    else
    {
        insertion_sort(arr);
    }
}

template<typename T>
void quick_sort(std::vector<T> & arr)
{
    quick_sort(arr, 0, arr.size()-1);
}

}//algo
