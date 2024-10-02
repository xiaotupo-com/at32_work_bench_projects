/**
 * @file xtp_common_inc.h
 * @author xiaotupo (xiaotupo@163.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef _XTP_COMMON_INC_H
#define _XTP_COMMON_INC_H

#define USING_XTP_MACRO_DEFINE

#if defined(USING_XTP_MACRO_DEFINE)
#include "xtp_macro_define.h"
#endif

#if defined(AT32)
#include "at32f435_437_wk_config.h"
#endif

#include "xtp_enum_class.h"
#include "xtp_gpio.h"

#endif // !_XTP_COMMON_INC_H