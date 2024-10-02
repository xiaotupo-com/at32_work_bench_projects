/**
 * @file xtp_delay.cpp
 * @author xiaotupo (xiaotupo@163.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "xtp_delay.h"

u16 fac_us;

/**
 * @brief 初始化 delay
 * 
 */
void delay_init()
{
    systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_DIV8);
    fac_us = SystemCoreClock / 8000000; // 288M / 8M = 36M
}

/**
 * @brief us 延时
 * 
 * @param us 
 */
void delay_us(u16 us)
{
    u32 temp;
    SysTick->LOAD = fac_us * us;
    SysTick->VAL = 0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; // 开始倒数
    do
    {
        temp = SysTick->CTRL;
    } while ((temp&0x01) &&!(temp&(1<<16)));
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; // 关闭计数器
    SysTick->VAL = 0x00; // 情况计数值
}

/**
 * @brief ms 延时
 * 
 * @param ms 
 */
void delay_ms(u16 ms)
{
    while(ms-- !=0)
    {
        delay_us(1000);
    }
}