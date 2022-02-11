#include <intrins.h>

#include "GT20L16S1Y.h"
#include "String.h"
#include "oled.h"

sbit BTN = P1 ^ 1;

void Show_String(u8 x, u8 y, u8 *text) {
  u8 i = 0;
  u8 addrHigh, addrMid, addrLow;
  u8 fontbuf[32];
  u32 fontaddr = 0;
  while (text[i] > 0x00) {
    if ((text[i] >= 0xb0) && (text[i] <= 0xf7) && (text[i + 1] >= 0xa1)) {
      //国标简体（GB2312）汉字在晶联讯字库IC中的地址由以下公式来计算：
      // Address = ((MSB - 0xB0) * 94 + (LSB - 0xA1)+ 846)*32 + BaseAdd;
      // BaseAdd=0
      //由于担心8位单片机有乘法溢出问题，所以分三部取地址
      fontaddr = (text[i] - 0xb0) * 94;
      fontaddr += (text[i + 1] - 0xa1) + 846;
      fontaddr = fontaddr * 32;

      addrHigh = (fontaddr & 0xff0000) >> 16; //地址的高8位,共24位
      addrMid = (fontaddr & 0xff00) >> 8;     //地址的中8位,共24位
      addrLow = (fontaddr & 0xff);            //地址的低8位,共24位

      S1Y_Get_Data(addrHigh, addrMid, addrLow, fontbuf, 32);
      //取32个字节的数据，存到"fontbuf[32]"
      OLED_Display_16x16(x, y, fontbuf);
      //显示汉字到LCD上，y为页地址，x为列地址，fontbuf[]为数据
      x += 16;
      i += 2;
    } else if ((text[i] >= 0xa1) && (text[i] <= 0xa3) &&
               (text[i + 1] >= 0xa1)) {

      fontaddr = (text[i] - 0xa1) * 94;
      fontaddr += (text[i + 1] - 0xa1);
      fontaddr = fontaddr * 32;

      addrHigh = (fontaddr & 0xff0000) >> 16;
      addrMid = (fontaddr & 0xff00) >> 8;
      addrLow = (fontaddr & 0xff);

      S1Y_Get_Data(addrHigh, addrMid, addrLow, fontbuf, 32);
      OLED_Display_16x16(x, y, fontbuf);
      x += 16;
      i += 2;
    } else if ((text[i] >= 0x20) && (text[i] <= 0x7e)) {
      u8 fontbuf[16];
      fontaddr = (text[i] - 0x20);
      fontaddr = (u32)(fontaddr * 16);
      fontaddr = (u32)(fontaddr + 0x3cf80);

      addrHigh = (fontaddr & 0xff0000) >> 16;
      addrMid = (fontaddr & 0xff00) >> 8;
      addrLow = fontaddr & 0xff;

      S1Y_Get_Data(addrHigh, addrMid, addrLow, fontbuf, 16);
      OLED_Display_8x16(x, y, fontbuf);
      x += 8;
      i += 1;
    } else {
      i++;
    }
  }
}

int random(){
	return 0;
}

int main(void) {
  u32 i = 0;
  OLED_Init(); //初始化OLED
  OLED_Clear();
  while (1) {
    Show_String(0, 0, String[i]);
    Show_String(0, 2, String[i + 1]);
    i += 2;
    if (i > len) {
      i = 0;
    }
    BTN = 1;
    while (BTN) {
      _nop_();
    }
		Delay_50ms(100);
    OLED_Clear();
  }
}
