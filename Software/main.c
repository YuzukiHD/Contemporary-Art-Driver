#include <intrins.h>
#include <math.h>

#include "GT20L16S1Y.h"
#include "String.h"
#include "oled.h"
#include "AHT21.h"

sbit BTN = P1 ^ 1;

bit MODE = 0;

#define IIC_Add_wr 0x70
#define IIC_RX_Length 7

u8 IIC_TX_Buffer[] = {0xAC, 0x33, 0x00};
u8 IIC_RX_Buffer[IIC_RX_Length] = {0x00};
u32 _ht = 0, _tt = 0;
int TEMP, HUMI;

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

void Clear_Data(void) {
  int i;

  for (i = 0; i < IIC_RX_Length; i++) {
    IIC_RX_Buffer[i] = 0x00;
  }
  _ht = _tt = 0;
}

void AHT21(void) {
	bit aaa, bbb;

  SCL = 1;
  SDA = 1;
  Clear_Data();
  WriteNByte(IIC_Add_wr, IIC_TX_Buffer, 3);
  Delay80ms();
	
	aaa = ReadNByte(IIC_Add_wr, IIC_RX_Buffer, IIC_RX_Length);
	bbb = CheckCRC(IIC_RX_Buffer, IIC_RX_Length);

  if (ReadNByte(IIC_Add_wr, IIC_RX_Buffer, IIC_RX_Length) != 0 ||
      CheckCRC(IIC_RX_Buffer, IIC_RX_Length) != 0) {
    _ht = (_ht | IIC_RX_Buffer[1]) << 8;
    _ht = (_ht | IIC_RX_Buffer[2]) << 8;
    _ht = (_ht | IIC_RX_Buffer[3]) >> 4;
    HUMI = ((int)_ht * 1.0 / 1024 / 1024) * 100;

    _tt = (_tt | IIC_RX_Buffer[3]) << 8;
    _tt = (_tt | IIC_RX_Buffer[4]) << 8;
    _tt = (_tt | IIC_RX_Buffer[5]);
    _tt = _tt & 0xfffff;
    TEMP = (((int)_tt * 1.0 / 1024 / 1024) * 200) - 50;
  }
}

int main(void) {
  u32 i = 0;
  u8 C_TEMP[4], C_HUMI[4];
  OLED_Init(); //初始化OLED
  OLED_Clear();
  while (1) {
    if (!MODE) {
      Show_String(0, 0, String[i]);
      Show_String(0, 2, String[i + 1]);
      i += 2;
      if (i > len) {
        i = 0;
      }
    } else {
      AHT21();
      Show_String(0, 0, HMD[0]);
			C_TEMP[0] = (TEMP % 1000) / 100 + '0';
			C_TEMP[1] = (TEMP % 100) / 10 + '0';
			C_TEMP[2] = TEMP % 10 + '0';
			C_TEMP[3] = '\0';
			Show_String(48, 0, C_TEMP);

			C_HUMI[0] = (HUMI % 1000) / 100 + '0';
			C_HUMI[1] = (HUMI % 100) / 10 + '0';
			C_HUMI[2] = HUMI % 10 + '0';
			C_HUMI[3] = '\0';
      Show_String(0, 2, HMD[1]);
			Show_String(48, 2, C_HUMI);
    }
    BTN = 1;
    while (BTN) {
      _nop_();
    }
    Delay_50ms(100);

    if (!BTN) {
      MODE = ~MODE;
    }

    OLED_Clear();
  }
}
