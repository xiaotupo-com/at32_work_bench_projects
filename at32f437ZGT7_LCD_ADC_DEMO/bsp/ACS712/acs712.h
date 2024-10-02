#ifndef __ACS712_H
#define __ACS712_H
/**
 * @file acs712.h
 * @author xiaotupo (xiaotupo@163.com)
 * @brief
 * @version 0.1
 * @date 2024-08-19
 *
 * @copyright Copyright (c) 2024
 *
 */
 
#include "at32f435_437.h"

#define OFFSET 1980

uint16_t adc_read();

class ACS712
{
public:
    uint16_t adc_read();
    uint16_t arithmetic_mean_filter();
    uint16_t middleAverageFilter();
    void setMax(float value);
    float getVoltage();
    float getCurrent();
};

#endif // !__ACS712_H