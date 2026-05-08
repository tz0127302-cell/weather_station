/**
 * @file lcd.c
 * @brief LCD显示屏驱动源文件
 * @details 实现LCD(ST7789)的管脚控制、数据发送、画图等功能
 * @author He
 * @date 2026-04-25
 */

#include "lcd.h"


/**
  * @brief LCD管脚初始化
  * @param  void
  * @retval void
  * @author He
            LCD_CS      ---     PC6     ---  片选
            LCD_DC      ---     PC7
            LCD_RST     ---     PC8     ---   通用推挽输出
                            拉低复位；拉高取消复位
            LCD_BLK     ---     PB12   
            LCD_SCL     ---     PB13
            LCD_SDA     ---     PB15    ---    发送数据线
  
  */

void Lcd_Port_Config(void)
{
    
    //打开GPIOB和GPIOD时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
    //IO管脚初始化
    GPIO_InitTypeDef GPIO_InitStruct={0};
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7  | GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC,&GPIO_InitStruct);
    
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_Init(GPIOB,&GPIO_InitStruct);
    //设置初始电平
    LCD_CS_H;
    LCD_BLK_OFF;
    LCD_RST_H;
 
}


/**
  * @brief LCD数据交换函数
  * @param  u8 data   发送的8bit数据
  * @retval void
  * @author He
    一次发送8bit，且高位先出    MODE0/MODE3
  */


void Lcd_TransferData(u8 data)
{
    
    for(u8 i=0;i<8;i++)
    {
        LCD_SCK_L;//拉低时钟线，主机向从机发送数据
        if(data & (0x80 >> i))
        {
            LCD_SDA_H;//拉高数据线，发送数据'1'
        }
        else
        {
            LCD_SDA_L;//拉低数据线，发送数据'0'
        }
        LCD_SCK_H;//拉高时钟线，从机接收数据
    }
}


/**
  * @brief LCD发送8bit数据/参数
  * @param  u8 data   发送的8bit数据
  * @retval void
  * @author He
    
  */

void Lcd_Send_Data(u8 data)
{
    
    LCD_DATA;//拉高DC，选择发送数据/参数
    LCD_CS_L;//拉低片选，建立通信
    Lcd_TransferData(data);//发送8bit数据
    LCD_CS_H;//拉高片选，结束通信
    
}




/**
  * @brief LCD发送8bit命令
  * @param  u8 cmd  发送命令
  * @retval void
  * @author He
    
  */

void Lcd_Send_Cmd(u8 cmd)
{
    LCD_CMD;//拉低DC,选择发送命令
    LCD_CS_L;//建立通信
    Lcd_TransferData(cmd);//发送命令
    LCD_CS_H;//结束通信
     
}



/**************************************************************************
 * 函 数 名：LCD_Init
 * 函数功能：LCD初始化
 * 入口参数：无
 * 出口参数：无
 * 备    注：包含GPIO初始化、SPI初始化和LCD参数设置，以及初始化显示
 ***************************************************************************/
void LCD_Init(void)
{
	//GPIO管脚初始化
    Lcd_Port_Config();

    LCD_RST_L;//复位
    delay_ms(100);
    LCD_RST_H;
    delay_ms(100);

	
	LCD_BLK_ON;	//打开背光
	delay_ms(100);
	
		//设置ST778VW芯片
	Lcd_Send_Cmd(0x11);
	delay_ms(100); //Delay 120ms
	Lcd_Send_Cmd(0X36);// Memory Access Control
	Lcd_Send_Data(0x70);
	Lcd_Send_Cmd(0X3A);
	Lcd_Send_Data(0X05);
	//--------------------------------ST7789S Frame rate setting-------------------------
	Lcd_Send_Cmd(0xb2);
	Lcd_Send_Data(0x0c);
	Lcd_Send_Data(0x0c);
	Lcd_Send_Data(0x00);
	Lcd_Send_Data(0x33);
	Lcd_Send_Data(0x33);
	Lcd_Send_Cmd(0xb7);
	Lcd_Send_Data(0x35);
	//---------------------------------ST7789S Power setting-----------------------------
	Lcd_Send_Cmd(0xbb);
	Lcd_Send_Data(0x35);
	Lcd_Send_Cmd(0xc0);
	Lcd_Send_Data(0x2c);
	Lcd_Send_Cmd(0xc2);
	Lcd_Send_Data(0x01);
	Lcd_Send_Cmd(0xc3);
	Lcd_Send_Data(0x12);
	Lcd_Send_Cmd(0xc4);
	Lcd_Send_Data(0x20);
	Lcd_Send_Cmd(0xc6);
	Lcd_Send_Data(0x0f);
	Lcd_Send_Cmd(0xca);
	Lcd_Send_Data(0x0f);
	Lcd_Send_Cmd(0xc8);
	Lcd_Send_Data(0x08);
	Lcd_Send_Cmd(0x55);
	Lcd_Send_Data(0x90);
	Lcd_Send_Cmd(0xd0);
	Lcd_Send_Data(0xa4);
	Lcd_Send_Data(0xa1);
	//--------------------------------ST7789S gamma setting------------------------------
	Lcd_Send_Cmd(0xe0);
	Lcd_Send_Data(0xd0);
	Lcd_Send_Data(0x00);
	Lcd_Send_Data(0x06);
	Lcd_Send_Data(0x09);
	Lcd_Send_Data(0x0b);
	Lcd_Send_Data(0x2a);
	Lcd_Send_Data(0x3c);
	Lcd_Send_Data(0x55);
	Lcd_Send_Data(0x4b);
	Lcd_Send_Data(0x08);
	Lcd_Send_Data(0x16);
	Lcd_Send_Data(0x14);
	Lcd_Send_Data(0x19);
	Lcd_Send_Data(0x20);
	Lcd_Send_Cmd(0xe1);
	Lcd_Send_Data(0xd0);
	Lcd_Send_Data(0x00);
	Lcd_Send_Data(0x06);
	Lcd_Send_Data(0x09);
	Lcd_Send_Data(0x0b);
	Lcd_Send_Data(0x29);
	Lcd_Send_Data(0x36);
	Lcd_Send_Data(0x54);
	Lcd_Send_Data(0x4b);
	Lcd_Send_Data(0x0d);
	Lcd_Send_Data(0x16);
	Lcd_Send_Data(0x14);
	Lcd_Send_Data(0x21);
	Lcd_Send_Data(0x20);
	Lcd_Send_Cmd(0x29);
	
    
    //清屏函数(颜色填充函数)
	Lcd_Clear(0,0,320,240,0xffff);
}




/**
  * @brief LCD发送16bit数据
  * @param  u16 data   发送的16bit数据
  * @retval void
  * @author He
    
  */

void Lcd_Send_Data16b(u16 data) 
{
    LCD_DATA;//拉高DC
    LCD_CS_L;//建立通信
    Lcd_TransferData(data>>8);//发送高8bit数据
    Lcd_TransferData(data);//发送低8bit数据
    LCD_CS_H;//结束通信
}


/**
  * @brief LCD设置显示范围函数(LCD挖坑)
  * @param  
            u16 xs    X轴起始坐标
            u16 ys    Y轴起始坐标
            u16 xe    X轴结束坐标
            u16 ye    Y轴结束坐标
  * @retval void
  * @author He
    
  */

void Lcd_SetPostion(u16 xs,u16 ys,u16 xe,u16 ye)
{
    //设置列地址
    Lcd_Send_Cmd(0x2A);//发送设置列地址命令
    Lcd_Send_Data16b(xs);//发送X轴起始坐标
    Lcd_Send_Data16b(xe-1);//发送X轴结束坐标
    //设置行地址
    Lcd_Send_Cmd(0x2B);//发送设置行地址命令
    Lcd_Send_Data16b(ys);//发送Y轴起始坐标
    Lcd_Send_Data16b(ye-1);//发送Y轴结束坐标
    //发送写帧存储器命令（告诉LCD驱动芯片，接下来我要开始发送颜色数据/显示数据）
    Lcd_Send_Cmd(0x2C);

}




/**
  * @brief LCD清屏函数（LCD向指定范围填充颜色）
  * @param  
            u16 xs    X轴起始坐标
            u16 ys    Y轴起始坐标
            u16 xe    X轴结束坐标
            u16 ye    Y轴结束坐标
            u16 color 填充颜色
  * @retval void
  * @author He
    
  */

void Lcd_Clear(u16 xs,u16 ys,u16 xe,u16 ye,u16 color)
{
    //①设置填充颜色范围
    Lcd_SetPostion(xs,ys,xe,ye);
    //②往设置的范围内每个像素点发送1个16bit颜色数据
    for(u32 i=0;i<(xe-xs)*(ye-ys);i++)
    {
       Lcd_Send_Data16b(color); 
    }
    
}


/**
  * @brief LCD显示图片函数
  * @param  
            u16 x    X轴起始坐标
            u16 y    Y轴起始坐标
            u8 *pic  指向1byte数据
  * @retval void
  * @author He
    
  */

void Lcd_DisplayPic(u16 x,u16 y,const u8 *pic)
{
    
    u16 w,h;
    //①在图像头数据中，获取图片宽度和高度
    w = pic[2] << 8 | pic[3];
    h = pic[4] << 8 | pic[5];
    //②设置图片显示范围/区域
    Lcd_SetPostion(x,y,x+w,y+h);
    //③开始填色
    for(u32 i=0;i<w*h;i++)
    {
        
        Lcd_Send_Data16b(pic[8+i*2] << 8 | pic[8+i*2+1]);
    }
 
}



/**
  * @brief LCD画点函数
  * @param  
            u16 x     X轴起始坐标
            u16 y     Y轴起始坐标
            u16 color 点的颜色
  * @retval void
  * @author He
    
  */

void Lcd_DrawPoint(u16 x,u16 y,u16 color)
{
    
    Lcd_SetPostion(x,y,x+1,y+1);//挖一个像素点大小的坑
    Lcd_Send_Data16b(color);
      
}



/**
  * @brief LCD显示单个英文字符函数
  * @param  
            u16 x               X轴起始坐标
            u16 y               Y轴起始坐标
            u16 font_color      字体颜色
            u16 bg_color        背景颜色
            u8  size            字体大小   16:8*16     24：12*24    32:16*32
            u8 eng             指向文字点阵数据
  * @retval void
  * @author He  
  */

void Lcd_DisplayChar(u16 x,u16 y,u16 font_color,u16 bg_color,u8 size,u8 eng)
{
    u8 i,j,k;
    u8 row_size = 0;//记录一行多少字节数据
    u32 addr = 0;
    u8 buff[256] = {0};
    /*
    知道这个字一行有多少字节（计算一行中有多少字节数据组成）
    size / 2 = 字的宽度 = 一行有多少位数据
    */
    row_size = size / 2 / 8;
    
    if(size / 2 % 8 != 0)
    {
        row_size += 1;
    }
    
    //1、计算不同大小英文字体字库偏移量
     switch(size)
    {
        case 16:addr+=0x00000000;break;
        case 24:addr+=0x00000806;break;
        case 32:addr+=0x0000200C;break;
    }
    
    //2、计算显示字体数据在字库的偏移量  英文
    addr += eng * row_size * size;
    
     //3、根据计算出来的偏移量（地址），读取W25Q64保存的字模数据
    Read_Data(addr,row_size*size,buff);
   
    //每一行需要判断8bit（8次），一共16行
    for(i=0;i<size;i++)//一共有多少行
    {
        for(k=0;k<row_size;k++)//一行中有多少字节
        {
           //宽度表示一行有多少个点（多少个位需要判断）
            for(j=0;j<8;j++)
            {
                if(buff[i*row_size+k] & 0x80 >> j)   //亮点
                {
                    Lcd_DrawPoint(x+j+k*8,y+i,font_color);
                }
                else                                //暗点
                {
                    Lcd_DrawPoint(x+j+k*8,y+i,bg_color);
                }
            }      
        }
        
    }
 
}


/**
  * @brief LCD显示单个汉字
  * @param  
            u16 x               X轴起始坐标
            u16 y               Y轴起始坐标
            u16 font_color      字体颜色
            u16 bg_color        背景颜色
            u8  size            字体大小   16:16*16     24：24*24    32:32*32
            u8 *chi             指向文字点阵数据
  * @retval void
  * @author He  
  */

void Lcd_DisplayChi(u16 x,u16 y,u16 font_color,u16 bg_color,u8 size,u8 *chi)
{
    
    u8 i,j,k;
    u8 row_size = 0;//记录一行多少字节数据
    u32 addr = 0;
    u8 buff[512]={0};
    
    row_size = size / 8;
    //1、计算不同大小中文字体字库偏移量
    switch(size)
    {
        case 16:addr+=0x00004012;break;
        case 24:addr+=0x00043E58;break;
        case 32:addr+=0x000D3A6E;break;
        
    }
    //2、计算显示字体数据在字库的偏移量
    addr += ((chi[0]-0xA1)*94 + (chi[1]-0xA1))*row_size*size;
    
    //3、根据计算出来的偏移量（地址），读取W25Q64保存的字模数据
    Read_Data(addr,row_size*size,buff);
    
    for(i=0;i<size;i++)//行
    {
        for(k=0;k<row_size;k++)
        {
          for(j=0;j<8;j++)
          {
             if(buff[i*row_size+k] & (0x80 >> j))
             {
                 Lcd_DrawPoint(x+j+k*8,y+i,font_color);//画亮点
             }
             else
             {
                 Lcd_DrawPoint(x+j+k*8,y+i,bg_color);//画暗点
             }    
          }          
        }
    }
  
}


/**
  * @brief LCD显示字符串
  * @param  
            u16 x               X轴起始坐标
            u16 y               Y轴起始坐标
            u16 font_color      字体颜色
            u16 bg_color        背景颜色
            u8  size            字体大小   16:8*16     24：12*24    32:16*32
            u8  str             u8 *str          指向显示字符串
  * @retval void
  * @author He  
  */

void Lcd_DisplayStr(u16 x,u16 y,u16 font_color,u16 bg_color,u8 size,u8 *str)
{

    while(*str != '\0')
    {
        
        if(*str >= 0xA1)//显示中文
        {   
            Lcd_DisplayChi(x,y,font_color,bg_color,size,str);
            x += size;
            str += 2;
            
        }
        else if(*str < 0xA1)//显示英文  "ABCD"
        {
            Lcd_DisplayChar(x,y,font_color,bg_color,size,*str);
            x += size / 2;
            str += 1;
            
        }      
    }
}




















