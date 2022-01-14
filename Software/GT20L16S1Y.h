#include "STC8G.h"

#ifndef _GT20L16S1Y_H_
#define _GT20L16S1Y_H_

#define u8 unsigned char
#define u32 unsigned long

sbit S1Y_CLK = P1 ^ 5;
sbit S1Y_SO = P1 ^ 3;
sbit S1Y_SI = P1 ^ 4;
sbit S1Y_CS = P1 ^ 2; /*字库IC接口定义Rom_CS就是字库IC的CS#*/

void S1Y_Write(u8 datu);
static u8 S1Y_Read();
void S1Y_Get_Data(u8 addrHigh, u8 addrMid, u8 addrLow, u8 *pBuff, u8 DataLen);

#endif