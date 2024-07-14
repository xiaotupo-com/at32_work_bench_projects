/**
 * @file st7789.h
 * @author xiaotupo (xiaotupo@163.com)
 * @brief
 * @version 0.1
 * @date 2024-07-06
 *
 * @copyright Copyright (c) 2024
 *
 * GPIO:
 * DC: Low 写命令，High 写数据
 * CS: 片选信号低电平有效，在读写数据前需要把 CS 写 0
 * RES: 在操作 ST7789 前需要先进行复位，首先输出 High 保持一段时间，然后输出 Low 保持一段时间，然后再拉高并保持一段时间，这样就完成了复位工作
 *
 */
#ifndef _XTP_ST7789_H
#define _XTP_ST7789_H

#include "xtp_common_inc.h"
#include <cstddef>

#define ST7789_WIDTH 320
#define ST7789_HEIGHT 240

/************** 颜色(RGB 5,6,5) **************/
#define LCD_DISP_RED                    0xF800       //先高字节，后低字节      
#define LCD_DISP_GREEN                  0x07E0
#define LCD_DISP_BLUE                   0x001F
#define LCD_DISP_WRITE                  0xFFFF
#define LCD_DISP_BLACK                  0x0000
#define LCD_DISP_GRAY                   0xEF5D
#define LCD_DISP_GRAY75                 0x39E7
#define LCD_DISP_GRAY50                 0x7BEF
#define LCD_DISP_GRAY25                 0xADB5
#define LCD_DISP_ALICEBLUE              0xEFBF  //艾丽丝蓝
#define LCD_DISP_ANTIQUEWHITE           0xF75A  //古典白色
#define LCD_DISP_AQUA                   0x07FF  //水绿色
#define LCD_DISP_AQUAMARINE             0x7ffa  //海蓝宝石色
#define LCD_DISP_AZURE                  0xEFFF  //蔚蓝色
#define LCD_DISP_BEIGE                  0xF7BB  //浅褐色
#define LCD_DISP_BLUEVIOLET             0x897B  //紫罗兰色
#define LCD_DISP_BROWN                  0xA145  //褐色
#define LCD_DISP_CADETBLUE              0x64F3  //军蓝色
#define LCD_DISP_CHARTREUSE             0x7FE0  //黄绿色
#define LCD_DISP_CHOCOLATE              0xD344  //巧克力色
#define LCD_DISP_CORAL                  0xFBEA  //珊瑚色
#define LCD_DISP_CORNFLOWERBLUE         0x64BD  //菊兰色
#define LCD_DISP_CORNSILK               0xFFBB  //玉米丝色
#define LCD_DISP_CRIMSON                0xd8a7  // 深红色
#define LCD_DISP_DARKBLUE               0x0011  //深蓝色
#define LCD_DISP_DARKCYAN               0x0451  //深青色
#define LCD_DISP_DEEPPINK               0xF8B2  //深粉色



class St7789
{
public:
    St7789() = delete;
    St7789(gpio_type *cs_gpio, uint16_t cs_pin, gpio_type *dc_gpio, uint16_t dc_pin, gpio_type *res_gpio, uint16_t res_pin);
    ~St7789() = default;
    St7789(const St7789 &other) = default;

public:
    void setBacklight(BlkState state) const;
	void clear(uint16_t color) const;
	void lcdInit() const;
    void drawPixel(u16 x, u16 y, u16 color);
    void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
    void drawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
    void writeChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor);
    void writeString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor);
    void writeNum(uint16_t x, uint16_t y, uint32_t num, const uint8_t len, FontDef font, uint16_t color, uint16_t bgcolor);
    void writeFloat(uint16_t x, uint16_t y, float num, const uint8_t len, FontDef font, uint16_t color, uint16_t bgcolor);
    void demo();

private:
    void reset() const;
    void setCSHigh() const;
    void setCSLow() const;
    void setDCHigh() const;
    void setDCLow() const;
    void sendCommand(const uint8_t command) const;
    void writeSingleByteData(uint8_t data) const;
    void sendData(uint8_t *data, size_t size) const;
    void write2Bytes(const uint16_t data) const;
    void setLcdAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) const;
    void displayOn() const;

private:
    const XTPGPio m_cs_gpio;
    const XTPGPio m_dc_gpio;
    const XTPGPio m_res_gpio;
};

#endif // _XTP_ST7789_H