#include <xc.h>
#include <stdlib.h>
#include "MB1.h"
#include "rs485.h"
#define MASTER		// ����Ԥ�������� ��ʶ���豸Ϊ485���豸

const unsigned char CODE[11]={0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x80};	// ���������
char cTest[10];		// 485ͨ�Ų�������
unsigned char IF_CODE;
unsigned char ASCII[15];
unsigned char i = 0, j;
extern char cOurAddrHigh, cOurAddrLow;
extern char cRs485RxChar;

void main(void)
{
	char cPacketReady;	// ���ݰ�׼����־
	char cLenOfData;	// ���ݰ�����
	char cCmd;			// �������� ѭ��������
	
	main_init();
	TM1620_Dis(0, 0, 0, 0);
	__delay_ms(20);
	
	// Just setup the network address	�洢������ַeeprom
	// write_eeprom(NET_ADDRESS_HIGH, 0x10);
	// write_eeprom(NET_ADDRESS_LOW, 0x10);
	// cOurAddrHigh = read_eeprom(NET_ADDRESS_HIGH );
	// cOurAddressLow = read_eeprom(NET_ADDRESS_LOW);
	while(1)
	{
		Rs485Initialise(0x00, 0x01);	// ��ʼ��485ͨ�� ָ���ӻ�
		Rs485SendPacket(0xA4, 1, 0x04);
		cPacketReady = Rs485Process();	// ���ݰ�Ч���ж�
		if (cPacketReady)				// �յ��ӻ����͵����ݺ�Ĵ���
		{
			Rs485GetPacket(&cCmd, &cLenOfData, &cTest[0]);	// ���ݽ�ȥ�ı���ͨ��ָ������²��� �������� ���ݳ��� ����
			if (cLenOfData != 0)		// ���ݳ��Ȳ���Ϊ0
			{
				TM1620_Dis(CODE[cTest[0]], CODE[cTest[1]], CODE[cTest[3]], CODE[cTest[4]]);
			}
			//Rs485SendPacket( SENSOR_ACK, 0, NULL );
			__delay_ms(250);
			Rs485Initialise(cOurAddrHigh, cOurAddrLow);	// ����485ģ��
		}
	}
}

void __interrupt() UsartInterrupt(void)	// �첽ͨ���жϷ���
{
	while(PIR1bits.RCIF)			// ѭ������жϱ�־
	{
		cRs485RxChar = RCREG;		// ��485ģ���������
		if (!(RCSTA & 6))			// ����Ƿ��н��մ���(֡����FERR �������OERR)
		{							// ��û�д��������������յ�������
			Rs485Decode();			// ����485��������
			RCSTAbits.CREN = 1;		// ʹ��CREN
		}
		else
		{
			RCSTAbits.CREN = 0;		// �ر�CREN
			cRs485RxChar = RCREG;	// ���¶�ȡ����
			cRs485RxChar = RCREG;	// �������״̬
			RCSTAbits.CREN = 1;		// ����CREN
		}
		PIR1bits.RCIF = 0;			// ��������жϱ�־
	}
}

void TM1620_Dis(char Thousandth, char Hundredth, char Tenth, char Unit)		// ��������
{
	INTCONbits.GIE = 0;
	TM1620Sencmd(GRID6_SEG8);		// ������ʾģʽ��6λ8��ģʽ
	TM1620Sencmd(AddrAutoAdd);		// ��������������õ�ַ�Զ���1ģʽ 0X40
    TM1620Sencmd(Addr00H);			// ���õ�ַ��00H��ʼ
	__delay_us(5);
	
    TM1620Sendata(Thousandth);			// 00H ż����ַ����ʾ����
    TM1620Sendata(0x00);			// 01H	������ַ����ʾλ��
	TM1620Sendata(Hundredth);			// 02H
    TM1620Sendata(0x00);			// 03H
	TM1620Sendata(Tenth);				// 04H
    TM1620Sendata(0x00);			// 05H
	TM1620Sendata(Unit);				// 06H
    TM1620Sendata(0x00);			// 07H
	__delay_ms(5);
	
	STB = 1;				// ��
	__delay_ms(1);
	STB = 0;				// ��
	__delay_ms(1);
	TM1620Sencmd(0x8F);		// ������ʾ�������ʾ������ռ�ձ�14/16.
	STB = 1;				// ��
	INTCONbits.GIE = 1;
}

void TM1620Sendata(char TM1620Data)		// ����һ���ֽ�
{
	char i;
	for(i = 0; i < 8; i++)
	{
		CLK = 0;			// CLK��
		DIN = 1 & (TM1620Data >> i);
		CLK = 1;			// CLK��
		__delay_us(3);
	}
}

void TM1620Sencmd(char com)		// ���Ϳ�������
{
	STB = 1;				// ��
	__delay_ms(1);
	STB = 0;				// ��
	__delay_ms(1);
	TM1620Sendata(com);
}
