/**
 * @file xtp_gpio.h
 * @author xiaotupo (xiaotupo@163.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef _XTP_GPIO_H
#define _XTP_GPIO_H

#include "xtp_common_inc.h"

class XTPGPio
{

public:
    // public -------------------------------------------------------------------------
#if defined(AT32)
    XTPGPio(gpio_type* gpiox, uint16_t pin);
    void setLevel(confirm_state level) const;

#endif

    // public -------------------------------------------------------------------------

private:
#if defined(AT32)
    gpio_type *m_gpiox;
    uint16_t m_pin;
#endif
};

#endif // !_XTP_GPIO_H