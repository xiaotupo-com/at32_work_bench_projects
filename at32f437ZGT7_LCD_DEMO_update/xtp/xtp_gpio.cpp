/**
 * @file xtp_gpio.cpp
 * @author xiaotupo (xiaotupo@163.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "xtp_gpio.h"

XTPGPio::XTPGPio(gpio_type* gpiox, uint16_t pin)
    : m_gpiox(gpiox), m_pin(pin)
{
}

void XTPGPio::setLevel(confirm_state level) const
{
#if defined(AT32)
    gpio_bits_write(m_gpiox, m_pin, level);
#endif
}