//****************************************************************//
//	������������AHT21B��AHT25��AM2301B��AM2108��AM2315C��DHT20,
//��Ƭ�� ��STC89C52RC
// ����  �����ڷ�����ʪ������  ������ 9600
// ����  ��11.0592Mhz (�û�ϵͳʱ���粻��12M �������غ궨�弰ע�͵���ʱʱ��)
// ���뻷��:  Keil5
// ��˾  �����ɵ���
//****************************************************************//

#include "reg52.h"
#include "stdio.h"
#include <intrins.h>

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long int u32;

//����IIC����
sbit SDA = P3 ^ 6;
sbit SCL = P3 ^ 7;

sfr     P3M0        =   0xb2;
sfr     P3M1        =   0xb1;

char buffer[100] = {0}; //���ڴ�ӡ�Ļ���
u8 CTDATA[7] = {0};     //����CRC��������
u8 CRC_WrongFlag = 0;   // CRC�����־λ   1:Wrong,0:True
u8 ACK_Flag = 0;        //Ӧ���־λ   1:ACK,0:NACK

/**************************��ʱ����*****************************
���߼������ǲ��:1��nop:6us, 2��Nop:7us, 3��Nop:8.5us, 4��Nop:9.5us,
                                                                5��Nop:10.5us,
6��Nop:11.5us
****************************************************************/
void delay_10us() //��ʱ10us
{
  _nop_();
  _nop_();
  _nop_();
  _nop_();
  _nop_();
}

//**********************************************
void delay_1ms() //��ʱ1ms�����߼������ǲ��i=84ʱ����ʱΪ1ms
{
  u8 i;
  for (i = 84; i > 0; i--) {
    delay_10us();
  }
}

//**********************************************
void delay_ms(u16 i) //��ʱ i ms������ʮ�־�׼����i����58ʱ���ᳬ��1ms
{
  for (; i > 0; i--) {
    delay_1ms();
  }
}

/**************************IIC����*****************************/

//����ʼλ sda=1->0
void I2C_Start() {
  SDA = 1;
  SCL = 1;
  delay_10us();
  SDA = 0;
  delay_10us();
  SCL = 0;
}
//��ֹͣλ sda=0->1
void Stop_I2C() {
  SDA = 0;
  SCL = 1;
  delay_10us();
  SDA = 1;
}
//��������ACK(����ack:sda=0)
void Send_ACK(void) { //����SDA ��Ϊ���
  SDA = 0;
  SCL = 0;
  delay_10us();
  SCL = 1;
  delay_10us();
  SCL = 0;
  SDA = 1;
}
//��������NACK(no_ack:sda=1��
void Send_NOT_ACK(void) { //����SDA ��Ϊ���
  SDA = 1;
  SCL = 0;
  delay_10us();
  SCL = 1;
  delay_10us();
  SDA = 1;
  SCL = 0;
}

// ��� SDA�Ƿ��ACK��ack:sda=1��Nack:sda=0��
u8 Receive_ACK(void) { //����SDA ��Ϊ����
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

void AHT20_WR_Byte(u8 Byte) //��AHT20дһ���ֽ�
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

u8 AHT20_RD_Byte(void) //��AHT20��ȡһ���ֽ�
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

u8 AHT20_Read_Status(void) //��ȡAHT20��״̬�Ĵ���
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

u8 AHT20_Read_Cal_Enable(void) //��ѯcal enableλ��û��ʹ��
{
  u8 val = 0; // ret = 0,
  val = AHT20_Read_Status();
  if ((val & 0x68) == 0x08)
    return 1;
  else
    return 0;
}

void AHT20_SendAC(void) //��AHT20����AC����
{

  I2C_Start();
  AHT20_WR_Byte(0x70);
  Receive_ACK();
  AHT20_WR_Byte(0xac); // 0xAC�ɼ�����
  Receive_ACK();
  AHT20_WR_Byte(0x33);
  Receive_ACK();
  AHT20_WR_Byte(0x00);
  Receive_ACK();
  Stop_I2C();
}

// CRCУ�����ͣ�CRC8/MAXIM
//����ʽ��X8+X5+X4+1
// Poly��0011 0001  0x31
//��λ�ŵ�����ͱ�� 1000 1100 0x8c
// C��ʵ���룺
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

void AHT20_Read_CTdata(u32 *ct) //û��CRCУ�飬ֱ�Ӷ�ȡAHT20���¶Ⱥ�ʪ������
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
  AHT20_SendAC(); //��AHT10����AC����
  delay_ms(80);   //��ʱ80ms����
  cnt = 0;
  while (((AHT20_Read_Status() & 0x80) ==
          0x80)) //ֱ��״̬bit[7]Ϊ0����ʾΪ����״̬����Ϊ1����ʾæ״̬
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
      AHT20_RD_Byte(); //״̬�֣���ѯ��״̬Ϊ0x98,��ʾΪæ״̬��bit[7]Ϊ1��״̬Ϊ0x1C������0x0C������0x08��ʾΪ����״̬��bit[7]Ϊ0
  Send_ACK();
  CTDATA[1] = Byte_2th = AHT20_RD_Byte(); //ʪ��
  Send_ACK();
  CTDATA[2] = Byte_3th = AHT20_RD_Byte(); //ʪ��
  Send_ACK();
  CTDATA[3] = Byte_4th = AHT20_RD_Byte(); //ʪ��/�¶�
  Send_ACK();
  CTDATA[4] = Byte_5th = AHT20_RD_Byte(); //�¶�
  Send_ACK();
  CTDATA[5] = Byte_6th = AHT20_RD_Byte(); //�¶�
  Send_NOT_ACK();
  Stop_I2C();

  RetuData = (RetuData | Byte_2th) << 8;
  RetuData = (RetuData | Byte_3th) << 8;
  RetuData = (RetuData | Byte_4th);
  RetuData = RetuData >> 4;
  ct[0] = RetuData; //ʪ��
  RetuData = 0;
  RetuData = (RetuData | Byte_4th) << 8;
  RetuData = (RetuData | Byte_5th) << 8;
  RetuData = (RetuData | Byte_6th);
  RetuData = RetuData & 0xfffff;
  ct[1] = RetuData; //�¶�
}

void AHT20_Read_CTdata_crc(u32 *ct) // CRCУ��󣬶�ȡAHT20���¶Ⱥ�ʪ������
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

  AHT20_SendAC(); //��AHT10����AC����
  delay_ms(80);   //��ʱ80ms����
  cnt = 0;
  while (((AHT20_Read_Status() & 0x80) ==
          0x80)) //ֱ��״̬bit[7]Ϊ0����ʾΪ����״̬����Ϊ1����ʾæ״̬
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
      AHT20_RD_Byte(); //״̬�֣���ѯ��״̬Ϊ0x98,��ʾΪæ״̬��bit[7]Ϊ1��״̬Ϊ0x1C������0x0C������0x08��ʾΪ����״̬��bit[7]Ϊ0
  Send_ACK();
  CTDATA[1] = Byte_2th = AHT20_RD_Byte(); //ʪ��
  Send_ACK();
  CTDATA[2] = Byte_3th = AHT20_RD_Byte(); //ʪ��
  Send_ACK();
  CTDATA[3] = Byte_4th = AHT20_RD_Byte(); //ʪ��/�¶�
  Send_ACK();
  CTDATA[4] = Byte_5th = AHT20_RD_Byte(); //�¶�
  Send_ACK();
  CTDATA[5] = Byte_6th = AHT20_RD_Byte(); //�¶�
  Send_ACK();
  CTDATA[6] = Byte_7th = AHT20_RD_Byte(); // CRC����
  Send_NOT_ACK();                         //ע��: ����Ƿ���NAK
  Stop_I2C();

  if (Calc_CRC8(CTDATA, 6) == Byte_7th) {
    CRC_WrongFlag = 0;
    RetuData = (RetuData | Byte_2th) << 8;
    RetuData = (RetuData | Byte_3th) << 8;
    RetuData = (RetuData | Byte_4th);
    RetuData = RetuData >> 4;
    ct[0] = RetuData; //ʪ��
    RetuData = 0;
    RetuData = (RetuData | Byte_4th) << 8;
    RetuData = (RetuData | Byte_5th) << 8;
    RetuData = (RetuData | Byte_6th);
    RetuData = RetuData & 0xfffff;
    ct[1] = RetuData; //�¶�
  } else {
    CRC_WrongFlag = 1; // CRC�����־λ
  }
}

void AHT20_Init(void) //��ʼ��AHT20
{
  I2C_Start();
  AHT20_WR_Byte(0x70);
  Receive_ACK();
  AHT20_WR_Byte(0xa8); // 0xA8����NOR����ģʽ
  Receive_ACK();
  AHT20_WR_Byte(0x00);
  Receive_ACK();
  AHT20_WR_Byte(0x00);
  Receive_ACK();
  Stop_I2C();

  delay_ms(10); //��ʱ10ms����

  I2C_Start();
  AHT20_WR_Byte(0x70);
  Receive_ACK();
  AHT20_WR_Byte(
      0xbe); // 0xBE��ʼ�����AHT20�ĳ�ʼ��������0xBE, AHT10�ĳ�ʼ��������0xE1
  Receive_ACK();
  AHT20_WR_Byte(0x08); //��ؼĴ���bit[3]��1��ΪУ׼���
  Receive_ACK();
  AHT20_WR_Byte(0x00);
  Receive_ACK();
  Stop_I2C();
  delay_ms(10); //��ʱ10ms����
}
void JH_Reset_REG(u8 addr) {

  u8 Byte_first, Byte_second, Byte_third;
  I2C_Start();
  AHT20_WR_Byte(0x70); //ԭ����0x70
  Receive_ACK();
  AHT20_WR_Byte(addr);
  Receive_ACK();
  AHT20_WR_Byte(0x00);
  Receive_ACK();
  AHT20_WR_Byte(0x00);
  Receive_ACK();
  Stop_I2C();

  delay_ms(5); //��ʱ5ms����
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

  delay_ms(10); //��ʱ10ms����
  I2C_Start();
  AHT20_WR_Byte(0x70); ///
  Receive_ACK();
  AHT20_WR_Byte(0xB0 | addr); ////�Ĵ�������
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
  delay_ms(500); //�ٸ��ϵ磬��ƷоƬ�ڲ�������Ҫʱ�䣬��ʱ100~500ms,����500ms
  if ((AHT20_Read_Status() & 0x18) != 0x18)
  {
    AHT20_Start_Init(); //���³�ʼ���Ĵ���
    delay_ms(10);
  }
  while (1) {
    AHT20_Read_CTdata_crc(CT_data); 
    c1 = ((float)CT_data[0] / 1024 / 1024) * 100; //����õ�ʪ��ֵc1�Ŵ�10��
    t1 = ((float)CT_data[1] / 1024 / 1024) * 200 - 50; //����õ��¶�ֵt1�Ŵ�10��
  }
}
