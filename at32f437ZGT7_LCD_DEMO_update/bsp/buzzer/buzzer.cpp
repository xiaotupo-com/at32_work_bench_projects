/**
 * @file buzzer.cpp
 * @author xiaotupo (xiaotupo@163.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "buzzer.h"
#include "xtp_delay.h"

/**
 * @brief Construct a new Buzzer:: Buzzer object
 * 
 * @param gpiox 
 * @param pin 
 */
Buzzer::Buzzer(gpio_type *gpiox, uint16_t pin)
    : xtpgpio(gpiox, pin)
{
}

/**
 * @brief 
 * 
 */
void Buzzer::off()
{
    xtpgpio.setLevel(FALSE);
}

/**
 * @brief 
 * 
 */
void Buzzer::on()
{
    xtpgpio.setLevel(TRUE);
}

/**
 * @brief 指定滴答间隔
 * 
 * @param width 单位 ms
 */
void Buzzer::on(int width)
{
    xtpgpio.setLevel(TRUE);
    delay_ms(width);
    xtpgpio.setLevel(FALSE);
    delay_ms(width);
}

/**
 * @brief 
 * 
 * @param on_time_ms 
 * @param off_time_ms 
 */
void Buzzer::on(int on_time_ms, int off_time_ms)
{
    xtpgpio.setLevel(TRUE);
    delay_ms(on_time_ms);
    xtpgpio.setLevel(FALSE);
    delay_ms(off_time_ms);
}