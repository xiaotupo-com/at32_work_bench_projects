#include <cstring>
#include "acs712.h"
#include "st7789.h"
#include "xtp_printf.h"
#include "xtp_common_inc.h"
#include "xtp_sort.h"
#include "xtp_utility.h"


extern St7789 st7789;
constexpr uint8_t window_size = 30;
uint16_t adc_values[window_size]; // 存储ADC采样值的数组
uint16_t temp_values[window_size-4];
uint64_t sum = 0;                 // 存储采样值总和
uint8_t index = 0;                // 数组索引

XTPGPio led2(LED2_GPIO_PORT, LED2_PIN);
/**
 * @brief 读取 adc 的值
 *
 * @return uint16_t
 */
uint16_t ACS712::adc_read()
{
    // 普通组（软件）触发使能
    adc_ordinary_software_trigger_enable(ADC1, TRUE);
    // ADC_OCCE_FLAG ：普通通道转换结束标志
    while (adc_flag_get(ADC1, ADC_OCCE_FLAG) == RESET)
    {
    }
    return adc_ordinary_conversion_data_get(ADC1);
}

/**
 * @brief 移动算术平均滤波算法
 *
 * @return uint16_t
 */
uint16_t ACS712::arithmetic_mean_filter()
{
    // 将新采样值加入数组，并更新总和
    adc_values[index] = adc_read(); // 读取ADC值
    // print_int(adc_values[index]);

    // 计算索引，实现循环队列的效果
    index = (index + 1) % window_size;

    // 如果数组已满，计算平均值
    if (index == 0)
    {
        sum = 0; // 清空 sum
        sum = XTP_Utility::u16_arr_sum(adc_values, window_size);
        return sum / window_size; // 返回平均值b n
    }

    return sum / window_size; // 如果数组未满，返回0或适当值
}

/**
 * @brief 计算最大电流
 *
 * @param value 单位为 A
 */
void ACS712::setMax(float value)
{
    // 1. 将最大电流值转换为 ADC 的值，1A 对应 100mV
    // 2. 用零电流时的 ADC 值加上最大电流对应的 ADC 值
    // 3. ADC 最小单位为：0.8mV
    //
    uint16_t max = value * 100 / 0.8 + 2255;

    if (arithmetic_mean_filter() > max)
    {
        st7789.writeString(4, 66, "Error", Font_11x18, LCD_DISP_CHARTREUSE, LCD_DISP_BLACK);
        led2.setLevel(FALSE);
    }
    else
    {
        st7789.writeString(4 + 11 * 6, 66, "Ok", Font_11x18, LCD_DISP_CHARTREUSE, LCD_DISP_BLACK);
        led2.setLevel(TRUE);
    }
}

/**
 * @brief 获取电压值 
 * 
 * @return float 
 */
float ACS712::getVoltage()
{
    return adc_read() * 3.3f / 4095;
}

/**
 * @brief 获取电流
 *
 * @return float
 */
float ACS712::getCurrent()
{
    // 用当前 adc 的值 - 1810 来获取变化的 ADC 值
    // 用变化的ADC 值转换成变化的电压(*0.8mV)
    float t = (arithmetic_mean_filter() - OFFSET) * 0.7976f / 1000; // 结果为 mV
    return t * 10;                                                  // 返回结果为 A
}

/**
 * @brief 
 * 
 * @return uint16_t 
 */
uint16_t ACS712::middleAverageFilter()
{
    // 将新采样值加入数组，并更新总和
    adc_values[index] = adc_read(); // 读取ADC值
    
    print_int(adc_values[index]);

    // 计算索引，实现循环队列的效果
    index = (index + 1) % window_size;

    // 如果数组已满，计算平均值
    if (index == 0)
    {
        XTP_Sort::bubble_sort(adc_values, window_size);
        memcpy(temp_values, adc_values+2, window_size-4);
        // print_int(XTP_Utility::u16_arr_sum(temp_values,window_size-4));
        return  XTP_Utility::u16_arr_sum(temp_values,window_size-4) / (uint16_t)(window_size-4); // 返回平均值
    }
    return XTP_Utility::u16_arr_sum(temp_values,window_size-4) / (uint16_t)(window_size-4); // 如果数组未满，返回0或适当值
} 