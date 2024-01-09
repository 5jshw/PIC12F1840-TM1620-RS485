#include <xc.h>
#include "MB1.h"

void main_init(void)
{
	// 485通信驱动
	T_DI = 0;	// 接收
	T_RO = 1;	// 发送
	DI = 0;
	RO = 0;
	
	// 数码管驱动芯片
	STB = 0;
	T_STB = 0;
	CLK = 0;
	T_CLK = 0;
	DIN = 0;
	T_DIN = 0;
	
	OSCCON = 0b01110000;	// 设置振荡器为8MHz
	ADCON0 = 0x10;			// 关闭AD转换
	ANSELA = 0;				// 关闭模拟输入
	OPTION_REG = 0x84;		// 关闭弱上拉 定时器预分频1:32
	APFCONbits.RXDTSEL = 1;	// 将RA5设定为接收
	APFCONbits.TXCKSEL = 1;	// 将RA4设定为发送
	TXSTAbits.BRGH = 1;		// 高速波特率
	BAUDCONbits.BRG16 = 1;	// 使用 16 位波特率发生器
	SPBRG = 207;			// 波特率 9600
	TXSTAbits.SYNC = 0;		// 异步模式
	RCSTAbits.SPEN = 1;		// 使能异步串口
	RCSTAbits.RX9 = 0;		// 关闭第9位接收
	TXSTAbits.TX9 = 0;		// 关闭第9位发送
	TXSTAbits.TXEN = 1;		// 发送使能
	RCSTAbits.CREN = 1;		// 连续接收使能
	
	// 中断设置
	INTCONbits.GIE = 1;		// 全局中断
	INTCONbits.PEIE = 1;	// 外设中断
	//PIE1bits.RCIE = 1;		// 接收中断
	
}

