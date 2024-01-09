
// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = OFF       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = OFF       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config DEBUG = OFF      // In-Circuit Debugger Mode (In-Circuit Debugger disabled, ICSPCLK and ICSPDAT are general purpose I/O pins)
#pragma config LVP = OFF         // Low-Voltage Programming Enable (Low-voltage programming enabled)

#define _XTAL_FREQ 8000000

void main_init(void);
void TM1620Sendata(char TM1620Data);
void TM1620Sencmd(char com);
void TM1620_Dis(char Thousandth, char Hundredth, char Tenth, char Unit);

#define AddrAutoAdd     0x40	// 写显示，自动累加地址 
#define Addr00H         0xC0	// 地址00H
#define GRID6_SEG8		0x02	// 6位8段数码管显示方式

#define STB			LATAbits.LATA0
#define T_STB		TRISAbits.TRISA0

#define CLK			LATAbits.LATA1
#define T_CLK		TRISAbits.TRISA1

#define DIN			LATAbits.LATA2
#define T_DIN		TRISAbits.TRISA2

#define RO			PORTAbits.RA4		// 发送
#define T_RO		TRISAbits.TRISA4

#define DI			PORTAbits.RA5		// 接收
#define T_DI		TRISAbits.TRISA5
