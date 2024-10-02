#include "xtp_utility.h"


uint16_t XTP_Utility::u16_arr_sum(uint16_t *arr, size_t size)
{
    uint16_t result = 0;

    for (int i=0; i<size; i++)
    {
        result += arr[i];
    }
    return result;
}