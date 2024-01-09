#include <xc.h>
#include <stdlib.h>
#include "MB1.h"
#include "rs485.h"
#define MASTER		// 定义预处理器宏 标识此设备为485主设备

const unsigned char CODE[11]={0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x80};	// 共阴数码管
char cTest[10];		// 485通信测试数据
unsigned char IF_CODE;
unsigned char ASCII[15];
unsigned char i = 0, j;
extern char cOurAddrHigh, cOurAddrLow;
extern char cRs485RxChar;

void main(void)
{
	char cPacketReady;	// 数据包准备标志
	char cLenOfData;	// 数据包长度
	char cCmd;			// 控制命令 循环计数器
	
	main_init();
	TM1620_Dis(0, 0, 0, 0);
	__delay_ms(20);
	
	// Just setup the network address	存储本机地址eeprom
	// write_eeprom(NET_ADDRESS_HIGH, 0x10);
	// write_eeprom(NET_ADDRESS_LOW, 0x10);
	// cOurAddrHigh = read_eeprom(NET_ADDRESS_HIGH );
	// cOurAddressLow = read_eeprom(NET_ADDRESS_LOW);
	while(1)
	{
		Rs485Initialise(0x00, 0x01);	// 初始化485通信 指定从机
		Rs485SendPacket(0xA4, 1, 0x04);
		cPacketReady = Rs485Process();	// 数据包效用判断
		if (cPacketReady)				// 收到从机发送的数据后的处理
		{
			Rs485GetPacket(&cCmd, &cLenOfData, &cTest[0]);	// 传递进去的变量通过指针接收新参数 控制命令 数据长度 数据
			if (cLenOfData != 0)		// 数据长度不能为0
			{
				TM1620_Dis(CODE[cTest[0]], CODE[cTest[1]], CODE[cTest[3]], CODE[cTest[4]]);
			}
			//Rs485SendPacket( SENSOR_ACK, 0, NULL );
			__delay_ms(250);
			Rs485Initialise(cOurAddrHigh, cOurAddrLow);	// 重置485模块
		}
	}
}

void __interrupt() UsartInterrupt(void)	// 异步通信中断服务
{
	while(PIR1bits.RCIF)			// 循环检测中断标志
	{
		cRs485RxChar = RCREG;		// 从485模块接收数据
		if (!(RCSTA & 6))			// 检查是否有接收错误(帧错误FERR 溢出错误OERR)
		{							// 若没有错误发生，则处理接收到的数据
			Rs485Decode();			// 调用485处理函数
			RCSTAbits.CREN = 1;		// 使能CREN
		}
		else
		{
			RCSTAbits.CREN = 0;		// 关闭CREN
			cRs485RxChar = RCREG;	// 重新读取数据
			cRs485RxChar = RCREG;	// 清除错误状态
			RCSTAbits.CREN = 1;		// 开启CREN
		}
		PIR1bits.RCIF = 0;			// 清除接收中断标志
	}
}

void TM1620_Dis(char Thousandth, char Hundredth, char Tenth, char Unit)		// 发送数据
{
	INTCONbits.GIE = 0;
	TM1620Sencmd(GRID6_SEG8);		// 设置显示模式，6位8段模式
	TM1620Sencmd(AddrAutoAdd);		// 设置数据命令，采用地址自动加1模式 0X40
    TM1620Sencmd(Addr00H);			// 设置地址从00H开始
	__delay_us(5);
	
    TM1620Sendata(Thousandth);			// 00H 偶数地址送显示数据
    TM1620Sendata(0x00);			// 01H	奇数地址送显示位数
	TM1620Sendata(Hundredth);			// 02H
    TM1620Sendata(0x00);			// 03H
	TM1620Sendata(Tenth);				// 04H
    TM1620Sendata(0x00);			// 05H
	TM1620Sendata(Unit);				// 06H
    TM1620Sendata(0x00);			// 07H
	__delay_ms(5);
	
	STB = 1;				// 高
	__delay_ms(1);
	STB = 0;				// 低
	__delay_ms(1);
	TM1620Sencmd(0x8F);		// 发送显示命令，打开显示并设置占空比14/16.
	STB = 1;				// 高
	INTCONbits.GIE = 1;
}

void TM1620Sendata(char TM1620Data)		// 发送一个字节
{
	char i;
	for(i = 0; i < 8; i++)
	{
		CLK = 0;			// CLK低
		DIN = 1 & (TM1620Data >> i);
		CLK = 1;			// CLK高
		__delay_us(3);
	}
}

void TM1620Sencmd(char com)		// 发送控制命令
{
	STB = 1;				// 高
	__delay_ms(1);
	STB = 0;				// 低
	__delay_ms(1);
	TM1620Sendata(com);
}

