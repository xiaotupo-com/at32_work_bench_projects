/* add user code begin Header */
/**
 **************************************************************************
 * @file     main.c
 * @brief    main program
 **************************************************************************
 *                       Copyright notice & Disclaimer
 *
 * The software Board Support Package (BSP) that is made available to
 * download from Artery official website is the copyrighted work of Artery.
 * Artery authorizes customers to use, copy, and distribute the BSP
 * software and its related documentation for the purpose of design and
 * development in conjunction with Artery microcontrollers. Use of the
 * software is governed by this copyright notice and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
 * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
 * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
 * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
 * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
 *
 **************************************************************************
 */
/* add user code end Header */

/* Includes ------------------------------------------------------------------*/
#include "at32f435_437_wk_config.h"
#include "wk_system.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */
#include <cstring>
#include "st7789.h"
#include "xtp_common_inc.h"
#include "xtp_delay.h"
#include "buzzer.h"
#include "xtp_printf.h"
#include "xtp_common_algorithm.h"
#include "acs712.h"

/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */

/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */
uint32_t num = 100;
float fnum = 0.0f;
St7789 st7789(LCD_CS_GPIO_PORT, LCD_CS_PIN, LCD_DC_GPIO_PORT, LCD_DC_PIN, LCD_RES_GPIO_PORT, LCD_RES_PIN);
ACS712 acs712;
// 定义 acd 数据变量
__IO uint16_t adc_ordinary_value;
__IO uint32_t dma1_trans_complete_flag = 0;

/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */

/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */

void show_adc_value()
{
    adc_ordinary_value = acs712.arithmetic_mean_filter();
    // adc_ordinary_value = acs712.middleAverageFilter();
    // adc_ordinary_value = acs712.adc_read();
	// print_int(adc_ordinary_value);
    st7789.writeString(0, 50, "adc->value: ", Font_11x18, LCD_DISP_ANTIQUEWHITE, LCD_DISP_BLACK);
    st7789.writeNum(strlen("adc->value: ") * Font_11x18.width, 50, adc_ordinary_value, 4, Font_11x18, LCD_DISP_AQUA, LCD_DISP_BLACK);
}

/* add user code end 0 */

/**
  * @brief main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  /* add user code begin 1 */

    /* add user code end 1 */

  /* system clock config. */
  wk_system_clock_config();

  /* config periph clock. */
  wk_periph_clock_config();

  /* nvic config. */
  wk_nvic_config();

  /* timebase config. */
  wk_timebase_init();

  /* init dma1 channel1 */
  wk_dma1_channel1_init();
  /* config dma channel transfer parameter */
  /* user need to modify define values DMAx_CHANNELy_XXX_BASE_ADDR and DMAx_CHANNELy_BUFFER_SIZE in at32xxx_wk_config.h */
  wk_dma_channel_config(DMA1_CHANNEL1, 
                        (uint32_t)&SPI1->dt, 
                        DMA1_CHANNEL1_MEMORY_BASE_ADDR, 
                        DMA1_CHANNEL1_BUFFER_SIZE);
  dma_channel_enable(DMA1_CHANNEL1, TRUE);

  /* init usart1 function. */
  wk_usart1_init();

  /* init spi1 function. */
  wk_spi1_init();

  /* init adc-common function. */
  wk_adc_common_init();

  /* init adc1 function. */
  wk_adc1_init();

  /* init gpio function. */
  wk_gpio_config();

  /* add user code begin 2 */
    delay_init(); // 延时函数优先初始化

    st7789.lcdInit();
    st7789.clear(LCD_DISP_BLACK);
    st7789.setBacklight(BlkState::on);

    st7789.drawRectangle(0, 0, ST7789_WIDTH - 1, 26, LCD_DISP_GREEN);
    st7789.writeString(4, 4, "https://github.com", Font_11x18, LCD_DISP_CHARTREUSE, LCD_DISP_BLACK);

    Buzzer buzzer(BUZZER_GPIO_PORT, BUZZER_PIN); // 创建蜂鸣器对象
    buzzer.on(150, 1000);

    /* add user code end 2 */

  while(1)
  {
    /* add user code begin 3 */
        // st7789.demo();
        num++;
        fnum += 0.12;
        // if (num > 99999)
        //     num = 0;
        // delay_ms(15);

        st7789.writeNum(strlen("https://github.com") * Font_11x18.width + 10, 4, num, 5, Font_11x18, LCD_DISP_ANTIQUEWHITE, LCD_DISP_BLACK);
        st7789.writeFloat(4, 30, fnum, 6, Font_11x18, LCD_DISP_ANTIQUEWHITE, LCD_DISP_BLACK);
        acs712.setMax(0.5f);
        show_adc_value();
        st7789.writeFloat(4, 98, acs712.getCurrent(), 6, Font_11x18, LCD_DISP_ANTIQUEWHITE, LCD_DISP_BLACK);
        /* add user code end 3 */
  }
}
