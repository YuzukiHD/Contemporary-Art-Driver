//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//中景园电子
//店铺地址：http://shop73023976.taobao.com/?spm=2013.1.0.0.M4PqC2
//
//  文 件 名   : main.c
//  版 本 号   : v2.0
//  作    者   : HuangKai
//  生成日期   : 2014-0101
//  最近修改   :
//  功能描述   : OLED 4接口演示例程(51系列)
//              说明:
//              ----------------------------------------------------------------
//              GND    电源地
//              VCC  接5V或3.3v电源
//              D0   P1^0（SCL）
//              D1   P1^1（SDA）
//-------------------------------------------------
// 修改历史   :
// 日    期   :
// 作    者   : HuangKai
// 修改内容   : 创建文件
// 版权所有，盗版必究。
// Copyright(C) 中景园电子2014/3/16
// All rights reserved
//******************************************************************************/。
#include "oled.h"


/**********************************************
//IIC Start
**********************************************/
void IIC_Start() {

  OLED_SCLK_Set();
  OLED_SDIN_Set();
  OLED_SDIN_Clr();
  OLED_SCLK_Clr();
}

/**********************************************
//IIC Stop
**********************************************/
void IIC_Stop() {
  OLED_SCLK_Set();
  OLED_SDIN_Clr();
  OLED_SDIN_Set();
}

void IIC_Wait_Ack() {
  OLED_SCLK_Set();
  OLED_SCLK_Clr();
}
/**********************************************
// IIC Write byte
**********************************************/

void Write_IIC_Byte(unsigned char IIC_Byte) {
  unsigned char i;
  unsigned char m, da;
  da = IIC_Byte;
  OLED_SCLK_Clr();
  for (i = 0; i < 8; i++) {
    m = da;
    m = m & 0x80;
    if (m == 0x80) {
      OLED_SDIN_Set();
    } else
      OLED_SDIN_Clr();
    da = da << 1;
    OLED_SCLK_Set();
    OLED_SCLK_Clr();
  }
}
/**********************************************
// IIC Write Command
**********************************************/
void Write_IIC_Command(unsigned char IIC_Command) {
  IIC_Start();
  Write_IIC_Byte(0x78); // Slave address,SA0=0
  IIC_Wait_Ack();
  Write_IIC_Byte(0x00); // write command
  IIC_Wait_Ack();
  Write_IIC_Byte(IIC_Command);
  IIC_Wait_Ack();
  IIC_Stop();
}
/**********************************************
// IIC Write Data
**********************************************/
void Write_IIC_Data(unsigned char IIC_Data) {
  IIC_Start();
  Write_IIC_Byte(0x78); // D/C#=0; R/W#=0
  IIC_Wait_Ack();
  Write_IIC_Byte(0x40); // write data
  IIC_Wait_Ack();
  Write_IIC_Byte(IIC_Data);
  IIC_Wait_Ack();
  IIC_Stop();
}
void OLED_WR_Byte(unsigned dat, unsigned cmd) {
  if (cmd) {

    Write_IIC_Data(dat);

  } else {
    Write_IIC_Command(dat);
  }
}


/***********************Delay****************************************/
void Delay_50ms(unsigned int Del_50ms) {
  unsigned int m;
  for (; Del_50ms > 0; Del_50ms--)
    for (m = 6245; m > 0; m--)
      ;
}

void Delay_1ms(unsigned int Del_1ms) {
  unsigned char j;
  while (Del_1ms--) {
    for (j = 0; j < 123; j++)
      ;
  }
}

//坐标设置

void OLED_Set_Pos(unsigned char x, unsigned char y) {
  OLED_WR_Byte(0xb0 + y, OLED_CMD);
  OLED_WR_Byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);
  OLED_WR_Byte((x & 0x0f), OLED_CMD);
}
//开启OLED显示
void OLED_Display_On(void) {
  OLED_WR_Byte(0X8D, OLED_CMD); // SET DCDC命令
  OLED_WR_Byte(0X14, OLED_CMD); // DCDC ON
  OLED_WR_Byte(0XAF, OLED_CMD); // DISPLAY ON
}
//关闭OLED显示
void OLED_Display_Off(void) {
  OLED_WR_Byte(0X8D, OLED_CMD); // SET DCDC命令
  OLED_WR_Byte(0X10, OLED_CMD); // DCDC OFF
  OLED_WR_Byte(0XAE, OLED_CMD); // DISPLAY OFF
}
//清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!
void OLED_Clear(void) {
  u8 i, n;
  for (i = 0; i < 8; i++) {
    OLED_WR_Byte(0xb0 + i, OLED_CMD); //设置页地址（0~7）
    OLED_WR_Byte(0x00, OLED_CMD);     //设置显示位置—列低地址
    OLED_WR_Byte(0x10, OLED_CMD);     //设置显示位置—列高地址
    for (n = 0; n < 128; n++)
      OLED_WR_Byte(0, OLED_DATA);
  } //更新显示
}
void OLED_On(void) {
  u8 i, n;
  for (i = 0; i < 8; i++) {
    OLED_WR_Byte(0xb0 + i, OLED_CMD); //设置页地址（0~7）
    OLED_WR_Byte(0x00, OLED_CMD);     //设置显示位置—列低地址
    OLED_WR_Byte(0x10, OLED_CMD);     //设置显示位置—列高地址
    for (n = 0; n < 128; n++)
      OLED_WR_Byte(1, OLED_DATA);
  } //更新显示
}

// m^n函数
u32 oled_pow(u8 m, u8 n) {
  u32 result = 1;
  while (n--)
    result *= m;
  return result;
}

void OLED_Display_16x16(u8 x, u8 y, u8 *dp) {
  u8 i, j;
  for (j = 0; j < 2; j++) {
    OLED_Set_Pos(x, y);
    for (i = 0; i < 16; i++) {
      //写数据到OLED,每写完一个8位的数据后列地址自动加1
      OLED_WR_Byte(*dp, OLED_DATA);
      dp++;
    }
    y++;
  }
}

//显示8x16点阵图像、ASCII, 或8x16点阵的自造字符、其他图标
void OLED_Display_8x16(u8 x, u8 y, u8 *dp) {
  u8 i, j;
  for (j = 0; j < 2; j++) {
    OLED_Set_Pos(x, y);
    for (i = 0; i < 8; i++) {
      //写数据到LCD,每写完一个8位的数据后列地址自动加1
      OLED_WR_Byte(*dp, OLED_DATA);
      dp++;
    }
    y++;
  }
}

//显示5*7点阵图像、ASCII, 或5x7点阵的自造字符、其他图标
void OLED_Display_5x7(u8 x, u8 y, u8 *dp) {
  u8 i;
  OLED_Set_Pos(x, y);
  for (i = 0; i < 6; i++) {
    OLED_WR_Byte(*dp, OLED_DATA);
    dp++;
  }
}


//初始化SSD1306
void OLED_Init(void) {
  Delay_1ms(100);
  OLED_WR_Byte(0xAE, OLED_CMD); //关闭显示

  OLED_WR_Byte(0x40, OLED_CMD); //---set low column address
  OLED_WR_Byte(0xB0, OLED_CMD); //---set high column address

  OLED_WR_Byte(0xC8, OLED_CMD); //-not offset

  OLED_WR_Byte(0x81, OLED_CMD); //设置对比度
  OLED_WR_Byte(0xff, OLED_CMD);

  OLED_WR_Byte(0xa1, OLED_CMD); //段重定向设置

  OLED_WR_Byte(0xa6, OLED_CMD); //

  OLED_WR_Byte(0xa8, OLED_CMD); //设置驱动路数
  OLED_WR_Byte(0x1f, OLED_CMD);

  OLED_WR_Byte(0xd3, OLED_CMD);
  OLED_WR_Byte(0x00, OLED_CMD);

  OLED_WR_Byte(0xd5, OLED_CMD);
  OLED_WR_Byte(0xf0, OLED_CMD);

  OLED_WR_Byte(0xd9, OLED_CMD);
  OLED_WR_Byte(0x22, OLED_CMD);

  OLED_WR_Byte(0xda, OLED_CMD);
  OLED_WR_Byte(0x02, OLED_CMD);

  OLED_WR_Byte(0xdb, OLED_CMD);
  OLED_WR_Byte(0x49, OLED_CMD);

  OLED_WR_Byte(0x8d, OLED_CMD);
  OLED_WR_Byte(0x14, OLED_CMD);

  OLED_WR_Byte(0xaf, OLED_CMD);
  OLED_Clear();
}
