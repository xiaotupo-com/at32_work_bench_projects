/**
 * @file buzzer.h
 * @author xiaotupo (xiaotupo@163.com)
 * @brief
 * @version 0.1
 * @date 2024-07-06
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef _BUZZER_H
#define _BUZZER_H
#include "xtp_gpio.h"

class Buzzer
{
public:
#if defined(AT32)
    Buzzer(gpio_type *gpiox, uint16_t pin);
#endif // AT32
    ~Buzzer() = default;

public:
    void off();
    void on();
    void on(int width);
    void on(int on_time_ms, int off_time_ms);

private:
    const XTPGPio xtpgpio;
};

#endif // !_BUZZER_H
