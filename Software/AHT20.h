#ifndef _AHT20_H_
#define _AHT20_H_


typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long int u32;


void Delay_N10us(u32 t);//延时函数
void SensorDelay_us(u32 t);//延时函数
void Delay_4us(void);		//延时函数
void Delay_5us(void);		//延时函数
void Delay_1ms(u32 t);	
void AHT20_Clock_Init(void);		//延时函数
void SDA_Pin_Output_High(void)  ; //将PB15配置为输出 ， 并设置为高电平， PB15作为I2C的SDA
void SDA_Pin_Output_Low(void);  //将P15配置为输出  并设置为低电平
void SDA_Pin_IN_FLOATING(void);  //SDA配置为浮空输入
void SCL_Pin_Output_High(void); //SCL输出高电平，P14作为I2C的SCL
void SCL_Pin_Output_Low(void); //SCL输出低电平
void Init_I2C_Sensor_Port(void); //初始化I2C接口,输出为高电平
void I2C_Start(void);		 //I2C主机发送START信号
void AHT20_WR_Byte(u8 Byte); //往AHT20写一个字节
u8 AHT20_RD_Byte(void);//从AHT20读取一个字节
u8 Receive_ACK(void);   //看AHT20是否有回复ACK
void Send_ACK(void)	;	  //主机回复ACK信号
void Send_NOT_ACK(void);	//主机不回复ACK
void Stop_I2C(void);	  //一条协议结束
u8 AHT20_Read_Status(void);//读取AHT20的状态寄存器
u8 AHT20_Read_Cal_Enable(void);  //查询cal enable位有没有使能
void AHT20_SendAC(void); //向AHT20发送AC命令
u8 Calc_CRC8(u8 *message,u8 Num);
void AHT20_Read_CTdata(u32 *ct); //没有CRC校验，直接读取AHT20的温度和湿度数据
void AHT20_Read_CTdata_crc(u32 *ct); //CRC校验后，读取AHT20的温度和湿度数据
void AHT20_Init(void);   //初始化AHT20
void JH_Reset_REG(u8 addr);///重置寄存器
void AHT20_Start_Init(void);///上电初始化进入正常测量状态

#endif


