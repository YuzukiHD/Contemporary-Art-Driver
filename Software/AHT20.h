#ifndef _AHT20_H_
#define _AHT20_H_


typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long int u32;


void Delay_N10us(u32 t);//��ʱ����
void SensorDelay_us(u32 t);//��ʱ����
void Delay_4us(void);		//��ʱ����
void Delay_5us(void);		//��ʱ����
void Delay_1ms(u32 t);	
void AHT20_Clock_Init(void);		//��ʱ����
void SDA_Pin_Output_High(void)  ; //��PB15����Ϊ��� �� ������Ϊ�ߵ�ƽ�� PB15��ΪI2C��SDA
void SDA_Pin_Output_Low(void);  //��P15����Ϊ���  ������Ϊ�͵�ƽ
void SDA_Pin_IN_FLOATING(void);  //SDA����Ϊ��������
void SCL_Pin_Output_High(void); //SCL����ߵ�ƽ��P14��ΪI2C��SCL
void SCL_Pin_Output_Low(void); //SCL����͵�ƽ
void Init_I2C_Sensor_Port(void); //��ʼ��I2C�ӿ�,���Ϊ�ߵ�ƽ
void I2C_Start(void);		 //I2C��������START�ź�
void AHT20_WR_Byte(u8 Byte); //��AHT20дһ���ֽ�
u8 AHT20_RD_Byte(void);//��AHT20��ȡһ���ֽ�
u8 Receive_ACK(void);   //��AHT20�Ƿ��лظ�ACK
void Send_ACK(void)	;	  //�����ظ�ACK�ź�
void Send_NOT_ACK(void);	//�������ظ�ACK
void Stop_I2C(void);	  //һ��Э�����
u8 AHT20_Read_Status(void);//��ȡAHT20��״̬�Ĵ���
u8 AHT20_Read_Cal_Enable(void);  //��ѯcal enableλ��û��ʹ��
void AHT20_SendAC(void); //��AHT20����AC����
u8 Calc_CRC8(u8 *message,u8 Num);
void AHT20_Read_CTdata(u32 *ct); //û��CRCУ�飬ֱ�Ӷ�ȡAHT20���¶Ⱥ�ʪ������
void AHT20_Read_CTdata_crc(u32 *ct); //CRCУ��󣬶�ȡAHT20���¶Ⱥ�ʪ������
void AHT20_Init(void);   //��ʼ��AHT20
void JH_Reset_REG(u8 addr);///���üĴ���
void AHT20_Start_Init(void);///�ϵ��ʼ��������������״̬

#endif


