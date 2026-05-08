/**
 * @file w25qxx.c
 * @brief W25Q64 FLASH存储器驱动源文件
 * @details 实W25Q64的读写、程、撤除撤除等操作
 * @author He
 * @date 2026-04-25
 */

#include "w25qxx.h"


/**
  * @brief  W25Q64初始化
  * @param  void
  * @retval void
  * @author He
  SD_CS          ---  PA4  ---  通用推挽输出
  */
void W25Qxx_Config(void)
{
    
    //打开GPIOC时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    //IO管脚初始化
    GPIO_InitTypeDef GPIO_InitStruct={0};
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_InitStruct);
    
    Spi1_Config();//SPI1初始化
    
    W25Q64_CS_H;//拉高片选
      
}


/**************************
函数功能：读W25Q64的ID
返回值：无
形参：无
函数说明：

***************************/
void W25q64_ReadID(void)
{
    u16 id = 0;
    
    W25Q64_CS_L;//拉低片选，建立通信
    
    Spi1_TransferData(0x90);//发送读ID指令
    
    Spi1_TransferData(0);
    Spi1_TransferData(0);
    Spi1_TransferData(0);
    
    id = Spi1_TransferData(0xff);//接收高八位数据
    id = id << 8 | Spi1_TransferData(0xff);//接收低八位数据并进行数据位移
    
    W25Q64_CS_H;//拉高片选，结束通信
    
    printf("id:0x%X\r\n",id);
    
    
}


/**************************
函数功能：写使能
返回值：无
形参：无
函数说明：
将状态寄存器WEL位置1
***************************/
void Write_Enable(void)
{
    
    W25Q64_CS_L;//拉低片选，建立通信
    Spi1_TransferData(0x06);//发送写使能指令
    W25Q64_CS_H;//拉高片选，结束通信
    
}

/**************************
函数功能：读状态寄存器1，并判断工作标志位
返回值：u8     0：表示W25Q64空闲     1：表示W25Q64忙碌
形参：无
函数说明：
判断8bit数据中最低位
***************************/

u8 Read_StatusReg(void)
{
    u8 status = 0;
    
    W25Q64_CS_L;//拉低片选，建立通信
    
    Spi1_TransferData(0x05);//发送读状态寄存器1指令
    
    status = Spi1_TransferData(0xff);//接收状态寄存器1的8bit数据
    
    W25Q64_CS_H;//拉高片选，结束通信
    
    if(status & 0x01)
        return 1;
    
    return 0;

}



/**************************
函数功能：扇区擦除
返回值：void
形参：u32 addr    地址
函数说明：
***************************/

void Sector_Erase(u32 addr)
{
    
    Write_Enable();//写使能
    
    while(Read_StatusReg());//等待不忙
    
    W25Q64_CS_L;//拉低片选，建立通信
    
    Spi1_TransferData(0x20);//发送扇区擦除指令
    
    Spi1_TransferData(addr >> 16);//发送地址的高8bit
    Spi1_TransferData(addr >> 8);//发送地址的中8bit
    Spi1_TransferData(addr & 0xff);//发送地址的低8bit
    
    W25Q64_CS_H;//拉高片选，W25Q64开始擦除
    
    while(Read_StatusReg());//等待不忙
}

/**************************
函数功能：半块擦除
返回值：void
形参：u32 addr    地址
函数说明：
***************************/

void HalfBlock_Erase(u32 addr)
{
    
    Write_Enable();//写使能
    
    while(Read_StatusReg());//等待不忙
    
    W25Q64_CS_L;//拉低片选，建立通信
    
    Spi1_TransferData(0x52);//发送半块擦除指令
    
    Spi1_TransferData(addr >> 16);//发送地址的高8bit
    Spi1_TransferData(addr >> 8);//发送地址的中8bit
    Spi1_TransferData(addr & 0xff);//发送地址的低8bit
    
    W25Q64_CS_H;//拉高片选，W25Q64开始擦除
    
    while(Read_StatusReg());//等待不忙
    
    
    
    
}


/**************************
函数功能：块擦除
返回值：void
形参：u32 addr    地址
函数说明：
***************************/

void Block_Erase(u32 addr)
{
    
    
    Write_Enable();//写使能
    
    while(Read_StatusReg());//等待不忙
    
    W25Q64_CS_L;//拉低片选，建立通信
    
    Spi1_TransferData(0xD8);//发送块擦除指令
    
    Spi1_TransferData(addr >> 16);//发送地址的高8bit
    Spi1_TransferData(addr >> 8);//发送地址的中8bit
    Spi1_TransferData(addr & 0xff);//发送地址的低8bit
    
    W25Q64_CS_H;//拉高片选，W25Q64开始擦除
    
    while(Read_StatusReg());//等待不忙
    
    
    
    
}


/**************************
函数功能：全片擦除
返回值：void
形参：void
函数说明：
***************************/

void Chip_Erase(void)
{
    
    
    Write_Enable();//写使能
    
    while(Read_StatusReg());//等待不忙
    
    W25Q64_CS_L;//拉低片选，建立通信
    
    Spi1_TransferData(0xC7);//发送全片擦除指令
    
    
    W25Q64_CS_H;//拉高片选，W25Q64开始擦除
    
    while(Read_StatusReg());//等待不忙
    
}  
    
    





/**************************
函数功能：页写
返回值：void
形参：
    u32 addr    地址
    u32 len     数据长度
    u8 *data    指向1byte数据
函数说明：
***************************/

void Page_Write(u32 addr,u32 len,u8 *data)
{
    
    Write_Enable();//写使能
    
    while(Read_StatusReg());//等待不忙
    
    W25Q64_CS_L;//拉低片选，建立通信
    
    Spi1_TransferData(0x02);//发送页编程指令
    
    Spi1_TransferData(addr >> 16);//发送地址的高8bit
    Spi1_TransferData(addr >> 8);//发送地址的中8bit
    Spi1_TransferData(addr & 0xff);//发送地址的低8bit
    
    while(len--)
    {
        
        Spi1_TransferData(*data);
        data++;
        
    }
    
    W25Q64_CS_H;//拉高片选，W25Q64开始写入数据
    
    while(Read_StatusReg());//等待不忙
    
    
}




/**************************
函数功能：读数据
返回值：void
形参：
    u32 addr    地址
    u32 len     数据长度
    u8 *data    指向1byte数据
函数说明：
***************************/

void Read_Data(u32 addr,u32 len,u8 *data)
{
    
    W25Q64_CS_L;
    
    Spi1_TransferData(0x03);//发送读数据指令
    
    Spi1_TransferData(addr >> 16);//发送地址的高8bit
    Spi1_TransferData(addr >> 8);//发送地址的中8bit
    Spi1_TransferData(addr & 0xff);//发送地址的低8bit
    
    while(len--)
    {
        *data++ = Spi1_TransferData(0xff);
        
    }
    
    W25Q64_CS_H;//拉高片选，结束通信
    
    
}




/**************************
函数功能：夸页写
返回值：void
形参：
    u32 addr    地址
    u32 len     数据长度
    u8 *data    指向1byte数据
函数说明：

"1234567"
***************************/

void W25Q64_CrossPageWrite(u32 addr,u32 len,u8 *data)
{
     u16 less_len = 0;
    
    less_len = 256 - addr % 256;
    
    if(len <= less_len)//当前页写得完
    {
        less_len = len;
    }
    
    while(1)
    {
        Page_Write(addr,less_len,data);
        //循环结束条件
        if(less_len == len)
        {
            break;
        }
        addr += less_len;//跨页
        len -= less_len;//总数据长度减去已写入数据大小
        data += less_len;//数据偏移已写入数据大小
        //判断跨完页后写不写得完
        if(len <= 256)
        {
            less_len = len;
        }
        else if(len > 256)
        {
            less_len = 256;
        } 
    }   
}




/**
  * @brief 字库更新
  * @param void
  * @retval void
  * @author He
  */
u8 file_flag = 0;  //0:还没有点击发送文件    1：点击了发送文件，文件正在发送，U1接收
u8 transfer_flag = 1;//0：表示烧录完成      1：表示正在烧录
void Font_Update(void)
{
    
    //1、擦除字库所需空间
    printf("正在擦除，请稍后\r\n");
    for(u8 i=0;i<30;i++)
    {
        Block_Erase(i*65536);
    }
    printf("擦除完成，请点击发送文件\r\n");
    
    //2、等待发送文件
    while(!file_flag);
    
    printf("正在烧录，请稍后\r\n");
    
    //3、等待字库烧录完成
    while(transfer_flag);
    
    printf("字库烧录完成\r\n");
     
}











