#include <xc.h>
#include "MB1.h"

void main_init(void)
{
	// 485ͨ������
	T_DI = 0;	// ����
	T_RO = 1;	// ����
	DI = 0;
	RO = 0;
	
	// ���������оƬ
	STB = 0;
	T_STB = 0;
	CLK = 0;
	T_CLK = 0;
	DIN = 0;
	T_DIN = 0;
	
	OSCCON = 0b01110000;	// ��������Ϊ8MHz
	ADCON0 = 0x10;			// �ر�ADת��
	ANSELA = 0;				// �ر�ģ������
	OPTION_REG = 0x84;		// �ر������� ��ʱ��Ԥ��Ƶ1:32
	APFCONbits.RXDTSEL = 1;	// ��RA5�趨Ϊ����
	APFCONbits.TXCKSEL = 1;	// ��RA4�趨Ϊ����
	TXSTAbits.BRGH = 1;		// ���ٲ�����
	BAUDCONbits.BRG16 = 1;	// ʹ�� 16 λ�����ʷ�����
	SPBRG = 207;			// ������ 9600
	TXSTAbits.SYNC = 0;		// �첽ģʽ
	RCSTAbits.SPEN = 1;		// ʹ���첽����
	RCSTAbits.RX9 = 0;		// �رյ�9λ����
	TXSTAbits.TX9 = 0;		// �رյ�9λ����
	TXSTAbits.TXEN = 1;		// ����ʹ��
	RCSTAbits.CREN = 1;		// ��������ʹ��
	
	// �ж�����
	INTCONbits.GIE = 1;		// ȫ���ж�
	INTCONbits.PEIE = 1;	// �����ж�
	//PIE1bits.RCIE = 1;		// �����ж�
	
}
