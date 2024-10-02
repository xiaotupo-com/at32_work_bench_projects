/**
 * @file xtp_delay.h
 * @author xiaotupo (xiaotupo@163.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef _XTP_DELAY_H
#define _XTP_DELAY_H

#include "xtp_common_inc.h"

void delay_init();
void delay_us(u16 us);
void delay_ms(u16 nms);

#endif // !_XTP_DELAY_H