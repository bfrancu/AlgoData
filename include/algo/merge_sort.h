#pragma once

#include <vector>

namespace algo
{

template<typename T>
void merge(std::vector<T> & arr, std::vector<T> & helper_arr, int left, int right, int right_end)
{
    int tmp_pos = left;
    int left_end = right - 1;
    int elements_count = right_end - left + 1;

    while (left <= left_end && right <= right_end)
    {
        if (arr[left] <= arr[right])
        {
            helper_arr[tmp_pos++] = std::move(arr[left++]);
        }
        else
        {
            helper_arr[tmp_pos++] = std::move(arr[right++]);
        }
    }

    while (left <= left_end)
    {
        helper_arr[tmp_pos++] = std::move(arr[left++]);
    }

    while (right <= right_end)
    {
        helper_arr[tmp_pos++] = std::move(arr[right++]);
    }


    for (int i = 0; i < elements_count; ++i, --right_end)
    {
        arr[right_end] = std::move(helper_arr[right_end]);
    }
}

template<typename T>
void merge_sort(std::vector<T> & arr, std::vector<T> & helper_arr, int left, int right)
{
    if (left < right)
    {
        int center = (right + left) / 2;
        merge_sort(arr, helper_arr, left, center);
        merge_sort(arr, helper_arr, center + 1, right);
        merge(arr, helper_arr, left, center+1, right);
    }
}

template <typename T>
void merge_sort(std::vector<T> & arr)
{
    std::vector<T> tmp_arr(arr);
    merge_sort(arr, tmp_arr, 0, arr.size() - 1);
}

}//algo
