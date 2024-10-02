/**
 * @file st7789.cpp
 * @author xiaotupo (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-07-06
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "st7789.h"
#include "xtp_delay.h"

#include <cstdio>
#include <cstring>

#define ABS(x) ((x) > 0 ? (x) : -(x))

/**
 * @brief       平方函数, m^n
 * @param       m: 底数
 * @param       n: 指数
 * @retval      m的n次方
 */
static uint32_t lcd_pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;

    while (n--)result *= m;

    return result;
}

/**
 * @brief Construct a new St 7 7 8 9:: St 7 7 8 9 object
 *
 * @param cs_gpio
 * @param cs_pin
 * @param dc_gpio
 * @param cd_pin
 * @param res_gpio
 * @param res_pin
 */
St7789::St7789(gpio_type *cs_gpio, uint16_t cs_pin, gpio_type *dc_gpio, uint16_t dc_pin, gpio_type *res_gpio, uint16_t res_pin)
    : m_cs_gpio(cs_gpio, cs_pin),
      m_dc_gpio(dc_gpio, dc_pin),
      m_res_gpio(res_gpio, res_pin)
{
}

/**
 * @brief 背光控制
 *
 * @param state
 */
void St7789::setBacklight(BlkState state) const
{
#if defined(AT32)
    if (state == BlkState::off)
    {
        gpio_bits_write(LCD_BLK_GPIO_PORT, LCD_BLK_PIN, FALSE);
    }

    if (state == BlkState::on)
    {
        gpio_bits_write(LCD_BLK_GPIO_PORT, LCD_BLK_PIN, TRUE);
    }

#endif // AT32
}

/**
 * @brief ST7789 复位
 *
 */
void St7789::reset() const
{
    m_res_gpio.setLevel(TRUE);
    delay_ms(1);
    m_res_gpio.setLevel(FALSE);
    delay_ms(1);
    m_res_gpio.setLevel(TRUE);
    delay_ms(1);
}

/**
 * @brief CS 引脚输出 1
 *
 */
void St7789::setCSHigh() const
{
    m_cs_gpio.setLevel(TRUE);
}

/**
 * @brief CS 引脚输出 0
 *
 */
void St7789::setCSLow() const
{
    m_cs_gpio.setLevel(FALSE);
}

/**
 * @brief DC 引脚输出 1
 *
 */
void St7789::setDCHigh() const
{
    m_dc_gpio.setLevel(TRUE);
}

/**
 * @brief DC 引脚输出 0
 *
 */
void St7789::setDCLow() const
{
    m_dc_gpio.setLevel(FALSE);
}

/**
 * @brief 写命令
 *          1. DC 引脚输出 Low
 * @param cmd
 */
void St7789::sendCommand(const uint8_t command) const
{

    setCSLow(); // cs 片选
    setDCLow(); // 写命令
    // 如果 发送缓存区不为空，则一直循环
    while (spi_i2s_flag_get(SPI1, SPI_I2S_TDBE_FLAG) == RESET)
    {
    }
    spi_i2s_data_transmit(SPI1, command);
    while (spi_i2s_flag_get(SPI1, SPI_I2S_TDBE_FLAG) == RESET)
    {
    }
    delay_us(1);
    setCSHigh(); // cs 释放
}

/**
 * @brief 写数据
 *          1. DC 引脚输出 High
 *
 * @param data
 */
void St7789::sendData(uint8_t *data, size_t size) const
{

    setCSLow();  // cs 片选
    setDCHigh(); // 写数据

#if defined(USE_DMA)
    /* config dma channel transfer parameter */
    /* user need to modify define values DMAx_CHANNELy_XXX_BASE_ADDR and DMAx_CHANNELy_BUFFER_SIZE in at32xxx_wk_config.h */
    dma_channel_enable(DMA1_CHANNEL1, FALSE);
    wk_dma_channel_config(DMA1_CHANNEL1, (uint32_t)&SPI1->dt, (uint32_t)data, size);
    dma_channel_enable(DMA1_CHANNEL1, TRUE);
    delay_us(10);

#else
    for (int i = 0; i < size; i++)
    {
        while (spi_i2s_flag_get(SPI1, SPI_I2S_TDBE_FLAG) == RESET)
        {
        }
        spi_i2s_data_transmit(SPI1, *data++);
        while (spi_i2s_flag_get(SPI1, SPI_I2S_TDBE_FLAG) == RESET)
        {
        }
        delay_us(1);
    }

#endif           // USE_DMA
    setCSHigh(); // cs 释放
}

/**
 * @brief 写单个字节数据
 *
 * @param data
 */
void St7789::writeSingleByteData(uint8_t data) const
{
    setCSLow();  // cs 片选
    setDCHigh(); // 写数据

    while (spi_i2s_flag_get(SPI1, SPI_I2S_TDBE_FLAG) == RESET)
    {
    }
    spi_i2s_data_transmit(SPI1, data);
    while (spi_i2s_flag_get(SPI1, SPI_I2S_TDBE_FLAG) == RESET)
    {
    }
    delay_us(1);

    setCSHigh(); // cs 释放
}

/**
 * @brief
 *
 * @param data
 */
void St7789::write2Bytes(const uint16_t data) const
{
    uint8_t _data[2] = {0};

    _data[0] = data >> 8;
    while (spi_i2s_flag_get(SPI1, SPI_I2S_TDBE_FLAG) == RESET)
        ;
    spi_i2s_data_transmit(SPI1, _data[0]);
    _data[1] = data;
    while (spi_i2s_flag_get(SPI1, SPI_I2S_TDBE_FLAG) == RESET)
        ;
    spi_i2s_data_transmit(SPI1, _data[1]);
}

/**
 * @brief Set the Lcd Address object
 *
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 */
void St7789::setLcdAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) const
{

    sendCommand(0x2a);
    {
        uint8_t _data[4];
        _data[0] = x1 >> 8;
        _data[1] = x1 & 0xFF;
        _data[2] = x2 >> 8;
        _data[3] = x2 & 0xFF;
        sendData(_data, sizeof(_data));
    }

    sendCommand(0x2b);
    {
        uint8_t _data[4];
        _data[0] = y1 >> 8;
        _data[1] = y1 & 0xFF;
        _data[2] = y2 & 0xFF;
        _data[3] = y2 & 0xFF;
        sendData(_data, sizeof(_data));
    }

    sendCommand(0x2C); // write ram
}

/**
 * @brief 初始化
 *
 */
void St7789::lcdInit() const
{
    setBacklight(BlkState::off); // 先关闭背光
    uint8_t _data;
    setCSHigh();
    reset();

    /************* display Setting *************/
    sendCommand(0x01); // 0x01 是软件复位指令
    delay_ms(50);
    sendCommand(0x11); // 0x11 是Sleep Out，退出休眠模式的命令
    delay_ms(500);

    // 修改此处，可以改变屏幕的显示方向，横屏，竖屏等
    sendCommand(0x36); // 0x36  ///< Memory Data Access Control
    /**
     * @brief 具体说明看 st7789 数据手册的 0x36 指令
     * 0x60: 屏幕横屏显示，左上角为原点
     * 0x00: 屏幕竖屏显示，左上角为原点
     */
    writeSingleByteData(0x60); // 00/50//40//C0/60

    sendCommand(0x3A);         //  0x3A     ///< Interface Pixel Format
    writeSingleByteData(0x05); /// 16bit/pixel

    /*********** Frame Rate Setting ***********/
    sendCommand(0xB2);         // 0xB2     ///< Porch Setting
    writeSingleByteData(0x0c); ///< Back porch in normal mode
    writeSingleByteData(0x0c); ///< Front porch in normal mode
    writeSingleByteData(0x00); ///< Disable separate porch control
    writeSingleByteData(0x33);
    writeSingleByteData(0x33);

    sendCommand(0xB7);         //  0xB7     ///< Gate Control
    writeSingleByteData(0x72); ///< VGH:13.26, VGL:-10.43

    sendCommand(0xBB);         // 0xBB     ///< VCOM Setting
    writeSingleByteData(0x3d); ///< VCOM: 1.425

    sendCommand(0xC0); /// 0xC0     ///< LCM Control
    writeSingleByteData(0x2c);

    sendCommand(0xC2); // 0xC2     ///< VDV and VRH Command Enable
    writeSingleByteData(0x01);
    writeSingleByteData(0xff);

    /* VRH Set */
    sendCommand(0xC3); // 0xC3
    writeSingleByteData(0x19);

    /* VDV Set */
    sendCommand(0xC4); // 0xC4
    writeSingleByteData(0x20);

    sendCommand(0xC6); //  0xC6     ///< Frame Rate Control in Normal Mode

    writeSingleByteData(0x0f); ///< 60Hz

    sendCommand(0xD0); // 0xD0     ///< Power Control 1
    writeSingleByteData(0xa4);
    writeSingleByteData(0xa1); ///< AVDD:6.8V, AVCL:-4.8V, VDDS:2.3V

    /* Positive Voltage Gamma Control */
    sendCommand(0xE0);
    writeSingleByteData(0xD0);
    writeSingleByteData(0x04);
    writeSingleByteData(0x0D);
    writeSingleByteData(0x11);
    writeSingleByteData(0x13);
    writeSingleByteData(0x2B);
    writeSingleByteData(0x3F);
    writeSingleByteData(0x54);
    writeSingleByteData(0x4C);
    writeSingleByteData(0x18);
    writeSingleByteData(0x0D);
    writeSingleByteData(0x0B);
    writeSingleByteData(0x1F);
    writeSingleByteData(0x23);

    /* Negative Voltage Gamma Control */
    sendCommand(0xE1);
    writeSingleByteData(0xe1);
    writeSingleByteData(0xD0);
    writeSingleByteData(0x04);
    writeSingleByteData(0x0C);
    writeSingleByteData(0x11);
    writeSingleByteData(0x13);
    writeSingleByteData(0x2C);
    writeSingleByteData(0x3F);
    writeSingleByteData(0x44);
    writeSingleByteData(0x51);
    writeSingleByteData(0x2F);
    writeSingleByteData(0x1F);
    writeSingleByteData(0x1F);
    writeSingleByteData(0x20);
    writeSingleByteData(0x23);
    sendCommand(0x29); ///< Display on
}

void St7789::clear(uint16_t color) const
{
    uint8_t _color[2];
    uint32_t i, j;
    setLcdAddress(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT);

    sendCommand(0X2C);
    
    for (i = 0; i < 240; i++)
    {
        for (j = 0; j < 320; j++)
        {
            writeSingleByteData(color >> 8);
            writeSingleByteData(color);
        }
    }
    
    displayOn(); /* 开LCD显示 */
}

void St7789::displayOn() const
{
    sendCommand(0x11); // 唤醒屏幕
    sendCommand(0x29); // Display on
}

void St7789::drawPixel(u16 x, u16 y, u16 color)
{
    setLcdAddress(x, y, x, y);

    writeSingleByteData(color >> 8);
    writeSingleByteData(color);
    displayOn();
}

void St7789::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    uint16_t swap;
    uint16_t steep = ABS(y1 - y0) > ABS(x1 - x0);
    if (steep)
    {
        swap = x0;
        x0 = y0;
        y0 = swap;

        swap = x1;
        x1 = y1;
        y1 = swap;
        //_swap_int16_t(x0, y0);
        //_swap_int16_t(x1, y1);
    }

    if (x0 > x1)
    {
        swap = x0;
        x0 = x1;
        x1 = swap;

        swap = y0;
        y0 = y1;
        y1 = swap;
        //_swap_int16_t(x0, x1);
        //_swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = ABS(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1)
    {
        ystep = 1;
    }
    else
    {
        ystep = -1;
    }

    for (; x0 <= x1; x0++)
    {
        if (steep)
        {
            drawPixel(y0, x0, color);
        }
        else
        {
            drawPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0)
        {
            y0 += ystep;
            err += dx;
        }
    }
}

/**
 * @brief
 *
 * @param x1 0 ~ WIDTH-1
 * @param y1 0 ~ HEIGHT-1
 * @param x2 0 ~ WIDTH-1
 * @param y2 0 ~ HEIGHT-1
 * @param color
 */
void St7789::drawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{

    drawLine(x1, y1, x2, y1, color);

    drawLine(x1, y1, x1, y2, color);

    drawLine(x1, y2, x2, y2, color);

    drawLine(x2, y1, x2, y2, color);
}

void St7789::demo()
{
    clear(LCD_DISP_RED);
    delay_ms(100);
    clear(LCD_DISP_GREEN);
    delay_ms(100);
    clear(LCD_DISP_BLUE);
    delay_ms(100);
    clear(LCD_DISP_WRITE);
    delay_ms(100);
    clear(LCD_DISP_BLACK);
    delay_ms(100);
    clear(LCD_DISP_GRAY);
    delay_ms(100);
    clear(LCD_DISP_GRAY75);
    delay_ms(100);
    clear(LCD_DISP_GRAY25);
    delay_ms(100);
    clear(LCD_DISP_ALICEBLUE);
    delay_ms(100);
    clear(LCD_DISP_ANTIQUEWHITE);
    delay_ms(100);
    clear(LCD_DISP_AQUA);
    delay_ms(100);
    clear(LCD_DISP_AQUAMARINE);
    delay_ms(100);
    clear(LCD_DISP_AZURE);
    delay_ms(100);
    clear(LCD_DISP_BEIGE);
    delay_ms(100);
    clear(LCD_DISP_BLUEVIOLET);
    delay_ms(100);
    clear(LCD_DISP_BROWN);
    delay_ms(100);
    clear(LCD_DISP_CADETBLUE);
    delay_ms(100);
}

void St7789::writeChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor)
{
    uint32_t i, b, j;
    setCSLow();
    setLcdAddress(x, y, x + font.width - 1, y + font.height - 1);

    for (i = 0; i < font.height; i++)
    {
        b = font.data[(ch - 32) * font.height + i];
        for (j = 0; j < font.width; j++)
        {
            if ((b << j) & 0x8000)
            {
                // uint8_t data[] = {color >> 8, color & 0xFF};
                uint8_t data[2];
                data[0] = color >> 8;
                data[1] = color & 0xFF;
                sendData(data, sizeof(data));
            }
            else
            {
                // uint8_t data[] = {bgcolor >> 8, bgcolor & 0xFF};
                uint8_t data[2];
                data[0] = bgcolor >> 8;
                data[1] = bgcolor & 0xFF;
                sendData(data, sizeof(data));
            }
        }
    }
    setCSHigh();
}

void St7789::writeString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor)
{
    setCSLow();
    while (*str)
    {
        if (x + font.width >= ST7789_WIDTH)
        {
            x = 0;
            y += font.height;
            if (y + font.height >= ST7789_HEIGHT)
            {
                break;
            }

            if (*str == ' ')
            {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }
        writeChar(x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }
    setCSHigh();
}

void St7789::writeNum(uint16_t x, uint16_t y, uint32_t num, const uint8_t len, FontDef font, uint16_t color, uint16_t bgcolor)
{
    char str[len];
    sprintf(str, "%d", num);
    writeString(x, y, str, font, color, bgcolor);
}

void St7789::writeFloat(uint16_t x, uint16_t y, float num, const uint8_t len, FontDef font, uint16_t color, uint16_t bgcolor)
{
    char str[len];
    sprintf(str, "%.2f", num);
    writeString(x, y, str, font, color, bgcolor);
}
