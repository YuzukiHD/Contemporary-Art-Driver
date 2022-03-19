//****************************************************************//
//	本例程适用于AHT21B，AHT25，AM2301B，AM2108，AM2315C，DHT20,
//单片机 ：STC89C52RC
// 功能  ：串口发送温湿度数据  波特率 9600
// 晶振  ：11.0592Mhz (用户系统时钟如不是12M 请更改相关宏定义及注释的延时时间)
// 编译环境:  Keil5
// 公司  ：奥松电子
//****************************************************************//

#include "reg52.h"
#include "stdio.h"
#include <intrins.h>

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long int u32;

//定义IIC引脚
sbit SDA = P3 ^ 6;
sbit SCL = P3 ^ 7;

sfr     P3M0        =   0xb2;
sfr     P3M1        =   0xb1;

char buffer[100] = {0}; //串口打印的缓存
u8 CTDATA[7] = {0};     //用于CRC传递数组
u8 CRC_WrongFlag = 0;   // CRC错误标志位   1:Wrong,0:True
u8 ACK_Flag = 0;        //应答标志位   1:ACK,0:NACK

/**************************延时部分*****************************
用逻辑分析仪测得:1个nop:6us, 2个Nop:7us, 3个Nop:8.5us, 4个Nop:9.5us,
                                                                5个Nop:10.5us,
6个Nop:11.5us
****************************************************************/
void delay_10us() //延时10us
{
  _nop_();
  _nop_();
  _nop_();
  _nop_();
  _nop_();
}

//**********************************************
void delay_1ms() //延时1ms，用逻辑分析仪测得i=84时，延时为1ms
{
  u8 i;
  for (i = 84; i > 0; i--) {
    delay_10us();
  }
}

//**********************************************
void delay_ms(u16 i) //延时 i ms，不是十分精准，当i大于58时误差会超过1ms
{
  for (; i > 0; i--) {
    delay_1ms();
  }
}

/**************************IIC部分*****************************/

//送起始位 sda=1->0
void I2C_Start() {
  SDA = 1;
  SCL = 1;
  delay_10us();
  SDA = 0;
  delay_10us();
  SCL = 0;
}
//送停止位 sda=0->1
void Stop_I2C() {
  SDA = 0;
  SCL = 1;
  delay_10us();
  SDA = 1;
}
//主机发送ACK(包含ack:sda=0)
void Send_ACK(void) { //设置SDA 口为输出
  SDA = 0;
  SCL = 0;
  delay_10us();
  SCL = 1;
  delay_10us();
  SCL = 0;
  SDA = 1;
}
//主机发送NACK(no_ack:sda=1）
void Send_NOT_ACK(void) { //设置SDA 口为输出
  SDA = 1;
  SCL = 0;
  delay_10us();
  SCL = 1;
  delay_10us();
  SDA = 1;
  SCL = 0;
}

// 检测 SDA是否回ACK（ack:sda=1；Nack:sda=0）
u8 Receive_ACK(void) { //设置SDA 口为输入
  u8 cnt = 0;
  SCL = 0;
  delay_10us();
  SCL = 1;
  delay_10us();
  while ((SDA == 1) && cnt < 100)
    cnt++;
  if (cnt == 100) {
    ACK_Flag = 0; // nack
  } else {
    ACK_Flag = 1; // ack
  }
  SCL = 0;
  delay_10us();
  return ACK_Flag;
}

void AHT20_WR_Byte(u8 Byte) //往AHT20写一个字节
{
  u8 Data, N, i;
  Data = Byte;
  i = 0x80;
  for (N = 0; N < 8; N++) {
    SCL = 0;
    delay_10us();
    if (i & Data) {
      SDA = 1;
    } else {
      SDA = 0;
    }
    SCL = 1;
    delay_10us();
    Data <<= 1;
  }
  SCL = 0;
  delay_10us();
}

u8 AHT20_RD_Byte(void) //从AHT20读取一个字节
{
  u8 Byte, i, a;
  Byte = 0;

  for (i = 0; i < 8; i++) {
    SCL = 0;
    delay_10us();
    SCL = 1;
    delay_10us();
    a = 0;
    if (SDA == 1)
      a = 1;
    Byte = (Byte << 1);
    Byte |= a;
    SCL = 0;
  }
  return Byte;
}

u8 AHT20_Read_Status(void) //读取AHT20的状态寄存器
{

  u8 Byte_first;
  I2C_Start();
  AHT20_WR_Byte(0x71);

  Receive_ACK();
  Byte_first = AHT20_RD_Byte();
  Send_NOT_ACK();
  Stop_I2C();
  return Byte_first;
}

u8 AHT20_Read_Cal_Enable(void) //查询cal enable位有没有使能
{
  u8 val = 0; // ret = 0,
  val = AHT20_Read_Status();
  if ((val & 0x68) == 0x08)
    return 1;
  else
    return 0;
}

void AHT20_SendAC(void) //向AHT20发送AC命令
{

  I2C_Start();
  AHT20_WR_Byte(0x70);
  Receive_ACK();
  AHT20_WR_Byte(0xac); // 0xAC采集命令
  Receive_ACK();
  AHT20_WR_Byte(0x33);
  Receive_ACK();
  AHT20_WR_Byte(0x00);
  Receive_ACK();
  Stop_I2C();
}

// CRC校验类型：CRC8/MAXIM
//多项式：X8+X5+X4+1
// Poly：0011 0001  0x31
//高位放到后面就变成 1000 1100 0x8c
// C现实代码：
u8 Calc_CRC8(u8 *message, u8 Num) {
  u8 i;
  u8 byte;
  u8 crc = 0xFF;
  for (byte = 0; byte < Num; byte++) {
    crc ^= (message[byte]);
    for (i = 8; i > 0; --i) {
      if (crc & 0x80)
        crc = (crc << 1) ^ 0x31;
      else
        crc = (crc << 1);
    }
  }
  return crc;
}

void AHT20_Read_CTdata(u32 *ct) //没有CRC校验，直接读取AHT20的温度和湿度数据
{
  volatile u8 Byte_1th = 0;
  volatile u8 Byte_2th = 0;
  volatile u8 Byte_3th = 0;
  volatile u8 Byte_4th = 0;
  volatile u8 Byte_5th = 0;
  volatile u8 Byte_6th = 0;
  volatile u8 Byte_7th = 0;
  u32 RetuData = 0;
  u16 cnt = 0;
  AHT20_SendAC(); //向AHT10发送AC命令
  delay_ms(80);   //延时80ms左右
  cnt = 0;
  while (((AHT20_Read_Status() & 0x80) ==
          0x80)) //直到状态bit[7]为0，表示为空闲状态，若为1，表示忙状态
  {
    delay_ms(2);
    if (cnt++ >= 100) {
      break;
    }
  }
  I2C_Start();
  AHT20_WR_Byte(0x71);
  Receive_ACK();
  CTDATA[0] = Byte_1th =
      AHT20_RD_Byte(); //状态字，查询到状态为0x98,表示为忙状态，bit[7]为1；状态为0x1C，或者0x0C，或者0x08表示为空闲状态，bit[7]为0
  Send_ACK();
  CTDATA[1] = Byte_2th = AHT20_RD_Byte(); //湿度
  Send_ACK();
  CTDATA[2] = Byte_3th = AHT20_RD_Byte(); //湿度
  Send_ACK();
  CTDATA[3] = Byte_4th = AHT20_RD_Byte(); //湿度/温度
  Send_ACK();
  CTDATA[4] = Byte_5th = AHT20_RD_Byte(); //温度
  Send_ACK();
  CTDATA[5] = Byte_6th = AHT20_RD_Byte(); //温度
  Send_NOT_ACK();
  Stop_I2C();

  RetuData = (RetuData | Byte_2th) << 8;
  RetuData = (RetuData | Byte_3th) << 8;
  RetuData = (RetuData | Byte_4th);
  RetuData = RetuData >> 4;
  ct[0] = RetuData; //湿度
  RetuData = 0;
  RetuData = (RetuData | Byte_4th) << 8;
  RetuData = (RetuData | Byte_5th) << 8;
  RetuData = (RetuData | Byte_6th);
  RetuData = RetuData & 0xfffff;
  ct[1] = RetuData; //温度
}

void AHT20_Read_CTdata_crc(u32 *ct) // CRC校验后，读取AHT20的温度和湿度数据
{
  volatile u8 Byte_1th = 0;
  volatile u8 Byte_2th = 0;
  volatile u8 Byte_3th = 0;
  volatile u8 Byte_4th = 0;
  volatile u8 Byte_5th = 0;
  volatile u8 Byte_6th = 0;
  volatile u8 Byte_7th = 0;
  u32 RetuData = 0;
  u16 cnt = 0;

  AHT20_SendAC(); //向AHT10发送AC命令
  delay_ms(80);   //延时80ms左右
  cnt = 0;
  while (((AHT20_Read_Status() & 0x80) ==
          0x80)) //直到状态bit[7]为0，表示为空闲状态，若为1，表示忙状态
  {
    delay_ms(1);
    if (cnt++ >= 100) {
      break;
    }
  }

  I2C_Start();

  AHT20_WR_Byte(0x71);
  Receive_ACK();
  CTDATA[0] = Byte_1th =
      AHT20_RD_Byte(); //状态字，查询到状态为0x98,表示为忙状态，bit[7]为1；状态为0x1C，或者0x0C，或者0x08表示为空闲状态，bit[7]为0
  Send_ACK();
  CTDATA[1] = Byte_2th = AHT20_RD_Byte(); //湿度
  Send_ACK();
  CTDATA[2] = Byte_3th = AHT20_RD_Byte(); //湿度
  Send_ACK();
  CTDATA[3] = Byte_4th = AHT20_RD_Byte(); //湿度/温度
  Send_ACK();
  CTDATA[4] = Byte_5th = AHT20_RD_Byte(); //温度
  Send_ACK();
  CTDATA[5] = Byte_6th = AHT20_RD_Byte(); //温度
  Send_ACK();
  CTDATA[6] = Byte_7th = AHT20_RD_Byte(); // CRC数据
  Send_NOT_ACK();                         //注意: 最后是发送NAK
  Stop_I2C();

  if (Calc_CRC8(CTDATA, 6) == Byte_7th) {
    CRC_WrongFlag = 0;
    RetuData = (RetuData | Byte_2th) << 8;
    RetuData = (RetuData | Byte_3th) << 8;
    RetuData = (RetuData | Byte_4th);
    RetuData = RetuData >> 4;
    ct[0] = RetuData; //湿度
    RetuData = 0;
    RetuData = (RetuData | Byte_4th) << 8;
    RetuData = (RetuData | Byte_5th) << 8;
    RetuData = (RetuData | Byte_6th);
    RetuData = RetuData & 0xfffff;
    ct[1] = RetuData; //温度
  } else {
    CRC_WrongFlag = 1; // CRC错误标志位
  }
}

void AHT20_Init(void) //初始化AHT20
{
  I2C_Start();
  AHT20_WR_Byte(0x70);
  Receive_ACK();
  AHT20_WR_Byte(0xa8); // 0xA8进入NOR工作模式
  Receive_ACK();
  AHT20_WR_Byte(0x00);
  Receive_ACK();
  AHT20_WR_Byte(0x00);
  Receive_ACK();
  Stop_I2C();

  delay_ms(10); //延时10ms左右

  I2C_Start();
  AHT20_WR_Byte(0x70);
  Receive_ACK();
  AHT20_WR_Byte(
      0xbe); // 0xBE初始化命令，AHT20的初始化命令是0xBE, AHT10的初始化命令是0xE1
  Receive_ACK();
  AHT20_WR_Byte(0x08); //相关寄存器bit[3]置1，为校准输出
  Receive_ACK();
  AHT20_WR_Byte(0x00);
  Receive_ACK();
  Stop_I2C();
  delay_ms(10); //延时10ms左右
}
void JH_Reset_REG(u8 addr) {

  u8 Byte_first, Byte_second, Byte_third;
  I2C_Start();
  AHT20_WR_Byte(0x70); //原来是0x70
  Receive_ACK();
  AHT20_WR_Byte(addr);
  Receive_ACK();
  AHT20_WR_Byte(0x00);
  Receive_ACK();
  AHT20_WR_Byte(0x00);
  Receive_ACK();
  Stop_I2C();

  delay_ms(5); //延时5ms左右
  I2C_Start();
  AHT20_WR_Byte(0x71); //
  Receive_ACK();
  Byte_first = AHT20_RD_Byte();
  Send_ACK();
  Byte_second = AHT20_RD_Byte();
  Send_ACK();
  Byte_third = AHT20_RD_Byte();
  Send_NOT_ACK();
  Stop_I2C();

  delay_ms(10); //延时10ms左右
  I2C_Start();
  AHT20_WR_Byte(0x70); ///
  Receive_ACK();
  AHT20_WR_Byte(0xB0 | addr); ////寄存器命令
  Receive_ACK();
  AHT20_WR_Byte(Byte_second);
  Receive_ACK();
  AHT20_WR_Byte(Byte_third);
  Receive_ACK();
  Stop_I2C();

  Byte_second = 0x00;
  Byte_third = 0x00;
}

void AHT20_Start_Init(void) {
  JH_Reset_REG(0x1b);
  JH_Reset_REG(0x1c);
  JH_Reset_REG(0x1e);
}

int main() {
  u32 CT_data[2];
  volatile float c1, t1;
  P3M0 = 0x00;
  P3M1 = 0x00;
  delay_ms(500); //①刚上电，产品芯片内部就绪需要时间，延时100~500ms,建议500ms
  if ((AHT20_Read_Status() & 0x18) != 0x18)
  {
    AHT20_Start_Init(); //重新初始化寄存器
    delay_ms(10);
  }
  while (1) {
    AHT20_Read_CTdata_crc(CT_data); 
    c1 = ((float)CT_data[0] / 1024 / 1024) * 100; //计算得到湿度值c1放大10倍
    t1 = ((float)CT_data[1] / 1024 / 1024) * 200 - 50; //计算得到温度值t1放大10倍
  }
}
