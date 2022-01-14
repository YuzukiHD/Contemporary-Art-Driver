#include "GT20L16S1Y.h"

void S1Y_Write(u8 datu) {
  u8 i;
  for (i = 0; i < 8; i++) {
    if (datu & 0x80)
      S1Y_SI = 1;
    else
      S1Y_SI = 0;
    datu = datu << 1;
    S1Y_CLK = 0;
    S1Y_CLK = 1;
  }
}

static u8 S1Y_Read() {
  u8 i;
  u8 ret_data = 0;
  S1Y_CLK = 1;
  for (i = 0; i < 8; i++) {
    S1Y_SO = 1;
    S1Y_CLK = 0;
    ret_data = ret_data << 1;
    if (S1Y_SO)
      ret_data = ret_data + 1;
    else
      ret_data = ret_data + 0;
    S1Y_CLK = 1;
  }
  return (ret_data);
}

/*从相关地址（addrHigh：地址高字节,addrMid：地址中字节,addrLow：地址低字节）中连续读出DataLen个字节的数据到
 * pBuff的地址*/
/*连续读取*/
void S1Y_Get_Data(u8 addrHigh, u8 addrMid, u8 addrLow, u8 *pBuff, u8 DataLen) {
  u8 i;
  S1Y_CS = 0;
  S1Y_CLK = 0;
  S1Y_Write(0x03);
  S1Y_Write(addrHigh);
  S1Y_Write(addrMid);
  S1Y_Write(addrLow);
  for (i = 0; i < DataLen; i++)
    *(pBuff + i) = S1Y_Read();
  S1Y_CS = 1;
}
