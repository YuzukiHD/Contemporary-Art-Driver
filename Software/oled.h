#include "STC8G.h"

#ifndef __OLED_H
#define __OLED_H

#define u8 unsigned char
#define u32 unsigned long

#define OLED_CMD 0  //写命令
#define OLED_DATA 1 //写数据
#define OLED_MODE 0

sbit OLED_SCL = P3 ^ 2;  //时钟 D0（SCLK?
sbit OLED_SDIN = P3 ^ 3; // D1（MOSI） 数据

#define OLED_CS_Clr() OLED_CS = 0
#define OLED_CS_Set() OLED_CS = 1

#define OLED_RST_Clr() OLED_RST = 0
#define OLED_RST_Set() OLED_RST = 1

#define OLED_DC_Clr() OLED_DC = 0
#define OLED_DC_Set() OLED_DC = 1

#define OLED_SCLK_Clr() OLED_SCL = 0
#define OLED_SCLK_Set() OLED_SCL = 1

#define OLED_SDIN_Clr() OLED_SDIN = 0
#define OLED_SDIN_Set() OLED_SDIN = 1

// OLED模式设置
// 0:4线串行模式
// 1:并行8080模式

#define SIZE 16
#define XLevelL 0x02
#define XLevelH 0x10
#define Max_Column 128
#define Max_Row 64
#define Brightness 0xFF
#define X_WIDTH 128
#define Y_WIDTH 64
//-----------------OLED端口定义----------------

// OLED控制用函数
void OLED_WR_Byte(unsigned dat, unsigned cmd);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(u8 x, u8 y, u8 t);
void OLED_Fill(u8 x1, u8 y1, u8 x2, u8 y2, u8 dot);
void OLED_Set_Pos(unsigned char x, unsigned char y);
void Delay_50ms(unsigned int Del_50ms);
void Delay_1ms(unsigned int Del_1ms);
void fill_picture(unsigned char fill_Data);
void Picture();
void IIC_Start();
void IIC_Stop();
void Write_IIC_Command(unsigned char IIC_Command);
void Write_IIC_Data(unsigned char IIC_Data);
void Write_IIC_Byte(unsigned char IIC_Byte);
void IIC_Wait_Ack();
void OLED_Display_16x16(u8 x, u8 y, u8 *dp);
void OLED_Display_8x16(u8 x, u8 y, u8 *dp);
void OLED_Display_5x7(u8 x, u8 y, u8 *dp);
#endif
