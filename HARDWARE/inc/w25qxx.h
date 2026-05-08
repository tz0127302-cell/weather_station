#ifndef _W25Qxx_H
#define _W25Qxx_H

/*******************ÎÄ¼þ°üº¬***********************/

#include "main.h"

/*******************ºê¶¨Òå***********************/

#define W25Q64_CS_H   (GPIO_SetBits(GPIOA,GPIO_Pin_4))
#define W25Q64_CS_L   (GPIO_ResetBits(GPIOA,GPIO_Pin_4))

/*******************º¯ÊýÉùÃ÷***********************/
extern u8 file_flag;
extern u8 transfer_flag;

void W25Qxx_Config(void);
void W25q64_ReadID(void);
void Sector_Erase(u32 addr);//ÉÈÇø²Á³ý
void HalfBlock_Erase(u32 addr);//°ë¿é²Á³ý
void Block_Erase(u32 addr);//¿é²Á³ý
void Chip_Erase(void);//È«Æ¬²Á³ý
void Page_Write(u32 addr,u32 len,u8 *data);//Ò³Ð´
void Read_Data(u32 addr,u32 len,u8 *data);//¶ÁÊý¾Ý
void W25Q64_CrossPageWrite(u32 addr,u32 len,u8 *data);//¿çÒ³Ð´
void Font_Update(void);

#endif
