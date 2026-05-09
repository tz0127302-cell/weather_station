/**
 * @file lcd.c
 * @brief LCD显示驱动源文件
 * @details 实现LCD(ST7789)的引脚控制、数据传输、画图等功能
 *          采用软件SPI方式驱动，通过GPIO模拟SPI时序与ST7789通信
 *          LCD分辨率为240x320，支持16位RGB565颜色格式
 * @author He
 * @date 2026-04-25
 */

#include "lcd.h"


/**
  * @brief LCD引脚初始化
  * @param  void
  * @retval void
  * @author He
  * @note   硬件引脚连接说明：
  *         LCD_CS      ---     PC6     ---  片选信号，低电平有效
  *         LCD_DC      ---     PC7     ---  数据/命令选择 (DC=0:命令, DC=1:数据)
  *         LCD_RST     ---     PC8     ---  复位信号，低电平复位
  *                                           通常由主机拉低再拉高完成复位
  *         LCD_BLK     ---     PB12    ---  背光控制，高电平点亮
  *         LCD_SCL     ---     PB13    ---  SPI时钟线(SCK)，软件模拟
  *         LCD_SDA     ---     PB15    ---  SPI数据线(MOSI)，软件模拟
  *
  *         注：本驱动使用软件SPI，仅需MOSI单向通信，无需MISO引脚
  */

void Lcd_Port_Config(void)
{

    // 使能GPIOB和GPIOD时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
    // IO引脚初始化配置
    GPIO_InitTypeDef GPIO_InitStruct={0};
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7  | GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;      // 推挽输出模式
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;      // 50MHz输出速度
    GPIO_Init(GPIOC,&GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_Init(GPIOB,&GPIO_InitStruct);
    // 设置初始化电平：片选拉高(禁用)，背光关闭，复位拉高(正常工作)
    LCD_CS_H;
    LCD_BLK_OFF;
    LCD_RST_H;

}

/**
  * @brief LCD数据发送底层函数（软件SPI位传输）
  * @param  u8 data   要发送的8bit数据
  * @retval void
  * @author He
  * @note   通过软件模拟SPI时序，一次发送8bit数据
  *         时钟模式为MODE0或MODE3：空闲时SCK为低电平，数据在SCK上升沿被采样
  *         高位先发送(MSB first)，逐位输出到SDA线
  */


void Lcd_TransferData(u8 data)
{

    for(u8 i=0;i<8;i++)
    {
        LCD_SCK_L;         // 拉低时钟线，准备开始传输一位数据
        if(data & (0x80 >> i))
        {
            LCD_SDA_H;     // 当前位为'1'，拉高数据线
        }
        else
        {
            LCD_SDA_L;     // 当前位为'0'，拉低数据线
        }
        LCD_SCK_H;         // 拉高时钟线，让LCD从机在上升沿锁存数据
    }
}


/**
  * @brief LCD发送8bit数据（数据模式）
  * @param  u8 data   要发送的8bit数据
  * @retval void
  * @author He
  * @note   先拉高DC引脚(数据模式)，再拉低CS使能从机，发送数据后拉高CS释放总线
  */

void Lcd_Send_Data(u8 data)
{

    LCD_DATA;          // 设置DC为高电平，选择数据模式
    LCD_CS_L;          // 拉低片选，使能LCD通信
    Lcd_TransferData(data);  // 发送8bit数据
    LCD_CS_H;          // 拉高片选，禁用LCD通信

}




/**
  * @brief LCD发送8bit命令
  * @param  u8 cmd  命令字节
  * @retval void
  * @author He
  * @note   先拉低DC引脚(命令模式)，再拉低CS使能从机，发送命令后拉高CS释放总线
  */

void Lcd_Send_Cmd(u8 cmd)
{
    LCD_CMD;           // 设置DC为低电平，选择命令模式
    LCD_CS_L;          // 拉低片选，使能LCD通信
    Lcd_TransferData(cmd);  // 发送命令字节
    LCD_CS_H;          // 拉高片选，禁用LCD通信

}



/**************************************************************************
 * 函 数 名：LCD_Init
 * 功能描述：LCD初始化
 * 输入参数：无
 * 输出参数：无
 * 说   明：包含GPIO初始化、SPI初始化、LCD寄存器配置以及初始化清屏显示
 ***************************************************************************/
void LCD_Init(void)
{
	// GPIO引脚初始化
    Lcd_Port_Config();

    LCD_RST_L;          // 拉低复位引脚，对LCD进行硬件复位
    delay_ms(100);
    LCD_RST_H;          // 拉高复位引脚，结束复位
    delay_ms(100);


	LCD_BLK_ON;	        // 打开背光
	delay_ms(100);

		// 初始化ST7789VW芯片寄存器序列，参考ST7789数据手册
	Lcd_Send_Cmd(0x11);         // 退出睡眠模式(SLPOUT)
	delay_ms(100);              // 等待120ms，退出睡眠后需要延时等待内部稳定
	Lcd_Send_Cmd(0X36);         // Memory Access Control (MADCTL)
	Lcd_Send_Data(0x70);       // 设置扫描方向：行交换、列交换、RGB顺序
	Lcd_Send_Cmd(0X3A);         // 接口像素格式(COLMOD)
	Lcd_Send_Data(0X05);       // 设置16位RGB565颜色格式(65K色)
	//--------------------------------ST7789S 帧率设置-------------------------
	Lcd_Send_Cmd(0xb2);        //  porch命令，设置前肩和后肩
	Lcd_Send_Data(0x0c);
	Lcd_Send_Data(0x0c);
	Lcd_Send_Data(0x00);
	Lcd_Send_Data(0x33);
	Lcd_Send_Data(0x33);
	Lcd_Send_Cmd(0xb7);        //  gate控制
	Lcd_Send_Data(0x35);
	//---------------------------------ST7789S 电源设置-----------------------------
	Lcd_Send_Cmd(0xbb);        //  VCOM反相控制
	Lcd_Send_Data(0x35);
	Lcd_Send_Cmd(0xc0);        // 电源控制1
	Lcd_Send_Data(0x2c);
	Lcd_Send_Cmd(0xc2);        // 电源控制2
	Lcd_Send_Data(0x01);
	Lcd_Send_Cmd(0xc3);        // 电源控制3
	Lcd_Send_Data(0x12);
	Lcd_Send_Cmd(0xc4);        // 电源控制4
	Lcd_Send_Data(0x20);
	Lcd_Send_Cmd(0xc6);        // 帧率控制
	Lcd_Send_Data(0x0f);
	Lcd_Send_Cmd(0xca);        // 电源控制
	Lcd_Send_Data(0x0f);
	Lcd_Send_Cmd(0xc8);        // 电源控制
	Lcd_Send_Data(0x08);
	Lcd_Send_Cmd(0x55);        // 亮度控制
	Lcd_Send_Data(0x90);
	Lcd_Send_Cmd(0xd0);        // 电压控制
	Lcd_Send_Data(0xa4);
	Lcd_Send_Data(0xa1);
	//--------------------------------ST7789S gamma校正设置------------------------------
	Lcd_Send_Cmd(0xe0);        // 正电压gamma校正
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
	Lcd_Send_Cmd(0xe1);        // 负电压gamma校正
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
	Lcd_Send_Cmd(0x29);        // 打开显示(DISPON)


    // 初始化清屏(白色填充整个屏幕)
	Lcd_Clear(0,0,320,240,0xffff);  // 0xFFFF = RGB565白色
}




/**
  * @brief LCD发送16bit数据
  * @param  u16 data   要发送的16bit数据
  * @retval void
  * @author He
  * @note   先将高8bit发送，再将低8bit发送，因为底层TransferData只支持8bit
  */

void Lcd_Send_Data16b(u16 data)
{
    LCD_DATA;            // 设置DC为高电平，选择数据模式
    LCD_CS_L;            // 拉低片选，使能LCD通信
    Lcd_TransferData(data>>8);  // 先发送高8位字节
    Lcd_TransferData(data);     // 再发送低8位字节(data>>8后传参仍是16位，但实际低8位有效)
    LCD_CS_H;            // 拉高片选，禁用LCD通信
}


/**
  * @brief LCD设置显示窗口范围(LCD画布裁剪区域)
  * @param
  *         u16 xs    X轴起始坐标
  *         u16 ys    Y轴起始坐标
  *         u16 xe    X轴结束坐标
  *         u16 ye    Y轴结束坐标
  * @retval void
  * @author He
  * @note   通过设置CASET(列地址)和RASET(行地址)命令来限定后续数据写入的区域
  *         之后发送的像素数据将只填充该窗口区域，超出部分自动换行
  *         结束坐标需减1，因为ST7789地址范围是包含边界的
  *         最后发送写存储器命令0x2C，通知LCD准备接收像素数据
  */

void Lcd_SetPostion(u16 xs,u16 ys,u16 xe,u16 ye)
{
    // 设置列地址范围(X轴)
    Lcd_Send_Cmd(0x2A);          // 列地址设置命令(CASET)
    Lcd_Send_Data16b(xs);        // 发送X轴起始地址
    Lcd_Send_Data16b(xe-1);      // 发送X轴结束地址(减1原因：地址从0开始)
    // 设置行地址范围(Y轴)
    Lcd_Send_Cmd(0x2B);          // 行地址设置命令(RASET)
    Lcd_Send_Data16b(ys);        // 发送Y轴起始地址
    Lcd_Send_Data16b(ye-1);      // 发送Y轴结束地址
    // 发送写存储器命令(告诉LCD准备好开始接收像素颜色数据)
    Lcd_Send_Cmd(0x2C);

}




/**
  * @brief LCD区域填充函数：将LCD指定范围内全部填充为指定颜色
  * @param
  *         u16 xs    X轴起始坐标
  *         u16 ys    Y轴起始坐标
  *         u16 xe    X轴结束坐标
  *         u16 ye    Y轴结束坐标
  *         u16 color 填充颜色(RGB565格式)
  * @retval void
  * @author He
  * @note   先通过Lcd_SetPostion设置填充区域，然后循环发送像素点颜色数据
  *         每个像素点占16bit(RGB565)，总循环次数 = 宽度 x 高度
  */

void Lcd_Clear(u16 xs,u16 ys,u16 xe,u16 ye,u16 color)
{
    // 设置待填充颜色的窗口范围
    Lcd_SetPostion(xs,ys,xe,ye);
    // 在设定的范围内，每个像素点发送1个16bit颜色值
    for(u32 i=0;i<(xe-xs)*(ye-ys);i++)
    {
       Lcd_Send_Data16b(color);
    }

}


/**
  * @brief LCD显示图片函数
  * @param
  *         u16 x      X轴起始坐标
  *         u16 y      Y轴起始坐标
  *         u8 *pic    指向图片数据的指针
  * @retval void
  * @author He
  * @note   图片数据格式：前6字节为信息头
  *         [0-1]: 保留字节
  *         [2-3]: 图片宽度(16bit大端)
  *         [4-5]: 图片高度(16bit大端)
  *         [6-7]: 保留字节
  *         [8+]:  RGB565像素数据(每个像素2字节)
  */

void Lcd_DisplayPic(u16 x,u16 y,const u8 *pic)
{

    u16 w,h;
    // 从图片数据头中提取图片宽度和高度(大端存储)
    w = pic[2] << 8 | pic[3];
    h = pic[4] << 8 | pic[5];
    // 设置图片显示窗口范围
    Lcd_SetPostion(x,y,x+w,y+h);
    // 逐像素发送颜色数据
    for(u32 i=0;i<w*h;i++)
    {
        // 每个像素2字节，大端存储，组合成16bit RGB565颜色值
        Lcd_Send_Data16b(pic[8+i*2] << 8 | pic[8+i*2+1]);
    }

}



/**
  * @brief LCD画点函数
  * @param
  *         u16 x     X轴坐标
  *         u16 y     Y轴坐标
  *         u16 color 点的颜色(RGB565格式)
  * @retval void
  * @author He
  * @note   设置大小为1x1像素的窗口，然后发送颜色数据
  *         注意：x和y必须确保在屏幕范围内(0-239, 0-319)
  */

void Lcd_DrawPoint(u16 x,u16 y,u16 color)
{

    Lcd_SetPostion(x,y,x+1,y+1);  // 设置1个像素点大小的窗口
    Lcd_Send_Data16b(color);      // 发送该像素的颜色值

}



/**
  * @brief LCD显示单个英文字符（从字库芯片W25Q64读取字模）
  * @param
  *         u16 x               X轴起始坐标
  *         u16 y               Y轴起始坐标
  *         u16 font_color      字体颜色
  *         u16 bg_color        背景颜色
  *         u8  size            字体大小
  *                             16: 8x16像素(字体宽度8,高度16)
  *                             24: 12x24像素(字体宽度12,高度24)
  *                             32: 16x32像素(字体宽度16,高度32)
  *         u8  eng             要显示的英文字符的ASCII码
  * @retval void
  * @author He
  * @note   英文字符宽度 = size/2，高度 = size
  *         字模存储在外部W25Q64 Flash芯片中，通过Read_Data函数读取
  *         存储方式为纵向取模(列行式)，每个位对应一个像素点，1为前景色，0为背景色
  *         对于宽度超过8像素的字符，每行需要多个字节存储
  */

void Lcd_DisplayChar(u16 x,u16 y,u16 font_color,u16 bg_color,u8 size,u8 eng)
{
    u8 i,j,k;
    u8 row_size = 0;         // 记录一行占多少字节
    u32 addr = 0;
    u8 buff[256] = {0};
    /*
    计算一个字模每行占用的字节数：
    size / 2 = 字符宽度(像素) = 一行有多少个位
    除以8转换为字节数
    */
    row_size = size / 2 / 8;

    if(size / 2 % 8 != 0)    // 如果宽度不是8的倍数，需要向上取整
    {
        row_size += 1;
    }

    // 1. 根据不同字体大小计算英文字库在W25Q64中的基地址偏移
     switch(size)
    {
        case 16:addr+=0x00000000;break;   // 8x16字库起始地址
        case 24:addr+=0x00000806;break;   // 12x24字库起始地址
        case 32:addr+=0x0000200C;break;   // 16x32字库起始地址
    }

    // 2. 计算要显示字符在字库中的实际地址偏移  英文字符
    addr += eng * row_size * size;  // 字符索引 * 每个字符占用的总字节数

     // 3. 根据计算出的地址偏移从W25Q64中读取字模数据
    Read_Data(addr,row_size*size,buff);

    // 逐行绘制：每行需要判断8bit，每列重复此过程
    for(i=0;i<size;i++)         // 行循环(字符高度)
    {
        for(k=0;k<row_size;k++) // 每行中的字节循环(宽度超过8位时)
        {
           // 内层循环判断一个字节中的每一位
            for(j=0;j<8;j++)
            {
                if(buff[i*row_size+k] & 0x80 >> j)   // 该位为1：绘制前景色(字体色)
                {
                    Lcd_DrawPoint(x+j+k*8,y+i,font_color);
                }
                else                                // 该位为0：绘制背景色
                {
                    Lcd_DrawPoint(x+j+k*8,y+i,bg_color);
                }
            }
        }

    }

}


/**
  * @brief LCD显示单个中文字符（从字库芯片W25Q64读取字模）
  * @param
  *         u16 x               X轴起始坐标
  *         u16 y               Y轴起始坐标
  *         u16 font_color      字体颜色
  *         u16 bg_color        背景颜色
  *         u8  size            字体大小
  *                             16: 16x16像素
  *                             24: 24x24像素
  *                             32: 32x32像素
  *         u8 *chi             指向中文字符的指针(GB2312编码，占2字节)
  * @retval void
  * @author He
  * @note   中文字符宽度 = 高度 = size(等宽字体)
  *         中文字库在W25Q64中的地址通过区位码计算：
  *         GB2312中文字符编码区为0xA1-0xFE，共计94区，每区94个字符
  *         偏移 = ((区码-0xA1)*94 + (位码-0xA1)) * 每字占用字节数
  */

void Lcd_DisplayChi(u16 x,u16 y,u16 font_color,u16 bg_color,u8 size,u8 *chi)
{

    u8 i,j,k;
    u8 row_size = 0;         // 记录一行占多少字节
    u32 addr = 0;
    u8 buff[512]={0};

    row_size = size / 8;     // 中文字符宽度等于高度，每行字节数 = size/8
    // 1. 根据不同字体大小计算中文字库在W25Q64中的基地址偏移
    switch(size)
    {
        case 16:addr+=0x00004012;break;   // 16x16中文字库起始地址
        case 24:addr+=0x00043E58;break;   // 24x24中文字库起始地址
        case 32:addr+=0x000D3A6E;break;   // 32x32中文字库起始地址

    }
    // 2. 根据汉字GB2312编码计算在字库中的实际地址偏移
    //    chi[0]为区码，chi[1]为位码，每个区94个字符
    addr += ((chi[0]-0xA1)*94 + (chi[1]-0xA1))*row_size*size;

    // 3. 根据计算出的地址偏移从W25Q64中读取字模数据
    Read_Data(addr,row_size*size,buff);

    for(i=0;i<size;i++)      // 行循环(字符高度)
    {
        for(k=0;k<row_size;k++)  // 每行中的字节循环
        {
          for(j=0;j<8;j++)       // 每个字节的位循环
          {
             if(buff[i*row_size+k] & (0x80 >> j))
             {
                 Lcd_DrawPoint(x+j+k*8,y+i,font_color);  // 1：绘制前景色
             }
             else
             {
                 Lcd_DrawPoint(x+j+k*8,y+i,bg_color);    // 0：绘制背景色
             }
          }
        }
    }

}


/**
  * @brief LCD显示字符串(自动区分中英文)
  * @param
  *         u16 x               X轴起始坐标
  *         u16 y               Y轴起始坐标
  *         u16 font_color      字体颜色
  *         u16 bg_color        背景颜色
  *         u8  size            字体大小
  *                             16: 16x16像素(中文) / 8x16像素(英文)
  *                             24: 24x24像素(中文) / 12x24像素(英文)
  *                             32: 32x32像素(中文) / 16x32像素(英文)
  *         u8  *str            指向要显示字符串的指针
  * @retval void
  * @author He
  * @note   通过判断字符编码自动区分中英文：
  *         >= 0xA1 为中文(GB2312编码，占2字节，宽度=size)
  *         < 0xA1  为英文(ASCII编码，占1字节，宽度=size/2)
  *         每显示完一个字符，x坐标向后移动相应宽度
  */

void Lcd_DisplayStr(u16 x,u16 y,u16 font_color,u16 bg_color,u8 size,u8 *str)
{

    while(*str != '\0')          // 遍历字符串直到结束符
    {

        if(*str >= 0xA1)         // 中文字符检测：GB2312编码的第一个字节 >= 0xA1
        {
            Lcd_DisplayChi(x,y,font_color,bg_color,size,str);  // 显示中文
            x += size;           // 中文字符宽度 = size
            str += 2;            // 中文占用2字节，指针前进2

        }
        else if(*str < 0xA1)     // 英文字符
        {
            Lcd_DisplayChar(x,y,font_color,bg_color,size,*str);  // 显示英文
            x += size / 2;       // 英文字符宽度 = size/2
            str += 1;            // 英文占用1字节，指针前进1

        }
    }
}













