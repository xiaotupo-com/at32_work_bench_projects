#ifndef __XTP_UTILITY_H
#define __XTP_UTILITY_H

#include "xtp_common_inc.h"

#define LENGTH_OF_ARRAY(arr) (sizeof(arr) / sizeof((arr)[0]))

class XTP_Utility
{
    public:
    static uint16_t u16_arr_sum(uint16_t *arr, size_t size);
};

#endif // __UTILITY_H
