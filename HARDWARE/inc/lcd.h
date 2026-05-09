#ifndef _LCD_H
#define _LCD_H

/**
 * @file lcd.h
 * @brief LCD液晶显示屏驱动头文件
 * @details 定义LCD屏幕的SPI控制引脚宏、RGB565颜色宏以及显示相关函数声明
 * @author He
 * @date 2026-04-25
 */

#include "main.h"

/* LCD数据线 (SDA/MOSI - 连接至GPIOB的Pin15引脚) */
#define LCD_SDA_H       (GPIO_SetBits(GPIOB,GPIO_Pin_15))  /* 数据线输出高电平 */
#define LCD_SDA_L       (GPIO_ResetBits(GPIOB,GPIO_Pin_15)) /* 数据线输出低电平 */

/* LCD时钟线 (SCK - 连接至GPIOB的Pin13引脚) */
#define LCD_SCK_H       (GPIO_SetBits(GPIOB,GPIO_Pin_13))  /* 时钟线输出高电平 */
#define LCD_SCK_L       (GPIO_ResetBits(GPIOB,GPIO_Pin_13)) /* 时钟线输出低电平 */

/* LCD片选信号 (CS - 连接至GPIOC的Pin6引脚) */
#define LCD_CS_H        (GPIO_SetBits(GPIOC,GPIO_Pin_6))   /* 片选拉高：取消选择LCD */
#define LCD_CS_L        (GPIO_ResetBits(GPIOC,GPIO_Pin_6)) /* 片选拉低：选中LCD，允许通信 */

/* LCD复位信号 (RST - 连接至GPIOC的Pin8引脚) */
#define LCD_RST_H        (GPIO_SetBits(GPIOC,GPIO_Pin_8))   /* 复位引脚拉高：退出复位状态 */
#define LCD_RST_L        (GPIO_ResetBits(GPIOC,GPIO_Pin_8)) /* 复位引脚拉低：进入复位状态 */

/* LCD数据/命令选择 (DC - 连接至GPIOC的Pin7引脚) */
#define LCD_DATA        (GPIO_SetBits(GPIOC,GPIO_Pin_7))   /* DC拉高：发送数据（RGB像素数据） */
#define LCD_CMD         (GPIO_ResetBits(GPIOC,GPIO_Pin_7)) /* DC拉低：发送命令（寄存器指令） */

/* LCD背光控制 (BLK - 连接至GPIOB的Pin12引脚) */
#define LCD_BLK_ON      (GPIO_SetBits(GPIOB,GPIO_Pin_12))  /* 打开背光：BLK引脚输出高电平 */
#define LCD_BLK_OFF     (GPIO_ResetBits(GPIOB,GPIO_Pin_12)) /* 关闭背光：BLK引脚输出低电平 */

/* RGB565颜色宏定义（16位色，格式为RRRRRGGGGGGBBBBB） */
#define WHITE              0xFFFF  /* 白色 */
#define BLACK              0x0000  /* 黑色 */
#define BLUE               0x001F  /* 蓝色 */
#define BRED               0XF81F  /* 红蓝色 (紫红) */
#define GRED               0XFFE0  /* 绿红色 (黄) */
#define GBLUE              0X07FF  /* 绿蓝色 (青) */
#define RED                0xF800  /* 红色 */
#define MAGENTA            0xF81F  /* 品红色 */
#define GREEN              0x07E0  /* 绿色 */
#define CYAN               0x7FFF  /* 青色 */
#define YELLOW             0xFFE0  /* 黄色 */
#define BROWN              0XBC40  /* 棕色 */
#define BRRED              0XFC07  /* 亮红色 */
#define GRAY               0X8430  /* 灰色 */
#define DARKBLUE           0X01CF  /* 深蓝色 */
#define LIGHTBLUE          0X7D7C  /* 浅蓝色 */
#define GRAYBLUE           0X5458  /* 灰蓝色 */
#define LIGHTGREEN         0X841F  /* 浅绿色 */
#define LGRAY              0XC618  /* 亮灰色 */
#define LGRAYBLUE          0XA651  /* 亮灰蓝色 */
#define LBBLUE             0X2B12  /* 淡蓝蓝色 */

/* 外部变量声明 */
extern const unsigned char gImage_1234[153608];  /* 开机图片数据数组（BMP格式，约150KB） */

/* 函数声明 */
void LCD_Init(void);                                                    /* LCD初始化函数：配置GPIO引脚、初始化LCD控制器、设置显示参数 */
void Lcd_Clear(u16 xs,u16 ys,u16 xe,u16 ye,u16 color);                  /* 区域填充函数：在指定矩形区域(xs,ys)到(xe,ye)内填充指定颜色 */
void Lcd_DisplayPic(u16 x,u16 y,const u8 *pic);                         /* 图片显示函数：在坐标(x,y)处显示指定的图片数据 */
void Lcd_DisplayChar(u16 x,u16 y,u16 font_color,u16 bg_color,u8 size,u8 eng); /* 字符显示函数：在指定位置显示一个英文字符，可设置字体颜色、背景色和字号 */
void Lcd_DisplayChi(u16 x,u16 y,u16 font_color,u16 bg_color,u8 size,u8 *chi); /* 汉字显示函数：在指定位置显示一个中文字符，可设置字体颜色、背景色和字号 */
void Lcd_DisplayStr(u16 x,u16 y,u16 font_color,u16 bg_color,u8 size,u8 *str); /* 字符串显示函数：在指定位置显示混合中英文字符串，可设置颜色和字号 */

#endif
