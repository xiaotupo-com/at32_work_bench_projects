#include "xtp_printf.h"
#include "at32f435_437.h"
#include <cstring>
#include <stdio.h>


// 定义缓存数组
#define UART_BUFFER_LEN 120
char uart_buffer[UART_BUFFER_LEN] = {0};

extern "C" void print_str(const char *str)
{
    for (uint8_t len = 0; len < strlen(str); len++)
    {
        while (usart_flag_get(USART1, USART_TDBE_FLAG) == RESET)
        {
        }
        usart_data_transmit(USART1, str[len]);
    }
}

extern "C" void print_int(const int value)
{
    sprintf(uart_buffer, "%d,", value);

    print_str(uart_buffer);
}