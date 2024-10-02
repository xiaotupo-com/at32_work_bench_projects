/**
 * @file xtp_sort.h
 * @author xiaotupo (xiaotupo.com)
 * @brief 排序算法集合
 * @version 0.1
 * @date 2024-08-25
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef __XTP_SORT_H
#define __XTP_SORT_H

#include "xtp_common_inc.h"

class XTP_Sort
{
public:
    /**
   * @brief 冒泡排序算法实现
   *
   * @tparam T
   * @param arr
   * @param size
   */
    template<typename T = int>
    static void bubble_sort(T *arr, size_t size)
    {
        int i, j;
        T temp;
        for (i = 0; i < size - 1; i++) {
            for (j = 0; j < size - i - 1; j++) {
                if (arr[j] > arr[j + 1]) {
                    temp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = temp;
                }
            }
        }
    }
};

#endif// __XTP_SORT_H
