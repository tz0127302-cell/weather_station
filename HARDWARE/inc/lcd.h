#ifndef _LCD_H
#define _LCD_H

/**
 * @file lcd.h
 * @brief LCD driver header
 */

#include "main.h"

/* LCD data line (SDA/MOSI - PB15) */
#define LCD_SDA_H       (GPIO_SetBits(GPIOB,GPIO_Pin_15))
#define LCD_SDA_L       (GPIO_ResetBits(GPIOB,GPIO_Pin_15))

/* LCD clock line (SCK - PB13) */
#define LCD_SCK_H       (GPIO_SetBits(GPIOB,GPIO_Pin_13))
#define LCD_SCK_L       (GPIO_ResetBits(GPIOB,GPIO_Pin_13))

/* LCD chip select (CS - PC6) */
#define LCD_CS_H        (GPIO_SetBits(GPIOC,GPIO_Pin_6))
#define LCD_CS_L        (GPIO_ResetBits(GPIOC,GPIO_Pin_6))

/* LCD reset (RST - PC8) */
#define LCD_RST_H        (GPIO_SetBits(GPIOC,GPIO_Pin_8))
#define LCD_RST_L        (GPIO_ResetBits(GPIOC,GPIO_Pin_8))

/* LCD data/command select (DC - PC7) */
#define LCD_DATA        (GPIO_SetBits(GPIOC,GPIO_Pin_7))
#define LCD_CMD         (GPIO_ResetBits(GPIOC,GPIO_Pin_7))

/* LCD backlight (BLK - PB12) */
#define LCD_BLK_ON      (GPIO_SetBits(GPIOB,GPIO_Pin_12))
#define LCD_BLK_OFF     (GPIO_ResetBits(GPIOB,GPIO_Pin_12))

/* RGB565 colors */
#define WHITE              0xFFFF
#define BLACK              0x0000
#define BLUE               0x001F
#define BRED               0XF81F
#define GRED               0XFFE0
#define GBLUE              0X07FF
#define RED                0xF800
#define MAGENTA            0xF81F
#define GREEN              0x07E0
#define CYAN               0x7FFF
#define YELLOW             0xFFE0
#define BROWN              0XBC40
#define BRRED              0XFC07
#define GRAY               0X8430
#define DARKBLUE           0X01CF
#define LIGHTBLUE          0X7D7C
#define GRAYBLUE           0X5458
#define LIGHTGREEN         0X841F
#define LGRAY              0XC618
#define LGRAYBLUE          0XA651
#define LBBLUE             0X2B12

/* external variables */
extern const unsigned char gImage_1234[153608];

/* function declarations */
void LCD_Init(void);
void Lcd_Clear(u16 xs,u16 ys,u16 xe,u16 ye,u16 color);
void Lcd_DisplayPic(u16 x,u16 y,const u8 *pic);
void Lcd_DisplayChar(u16 x,u16 y,u16 font_color,u16 bg_color,u8 size,u8 eng);
void Lcd_DisplayChi(u16 x,u16 y,u16 font_color,u16 bg_color,u8 size,u8 *chi);
void Lcd_DisplayStr(u16 x,u16 y,u16 font_color,u16 bg_color,u8 size,u8 *str);

#endif
