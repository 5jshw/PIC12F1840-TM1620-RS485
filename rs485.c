#include <xc.h>
#include <stdlib.h>
#include "rs485.h"
#include "MB1.h"

// struct
char cOurAddrHigh, cOurAddrLow; // 这两个变量用于存储将发送数据的指定从机地址
char cRs485RxChar; // 这个变量用于临时存储从RS485接收到的单个字符
static char cRS485State; // 用于表示RS485数据处理的当前状态 这通常是一个状态机的一部分 用于跟踪数据包解码过程的进度

static char cStart; // 用于存储数据包的开始字符或标记
static char cNetAddrHigh, cNetAddrLow; // 这两个变量用于存储接收到的指定从机地址
static char cLenExpected; // 存储接收到的数据包中预期的数据长度
static char cCommand; // 存储数据包中的命令字节 这通常指定了要执行的操作或数据包的类型
static char c485Buf[64]; // 这是一个字符数组 用于存储接收到的数据包的数据部分
static char cRxCrcHigh, cRxCrcLow; // 这两个变量用于存储接收到的数据包的CRC校验和的高位和低位
static char cCalcCrcHigh, cCalcCrcLow; // 这两个变量用于存储计算出的CRC校验和的高位和低位 用于与接收到的CRC校验和进行比较
static char cBufPtr; // 一个指针（或索引） 用于跟踪在 c485Buf 数组中当前的写入位置
static char cError; // 用于存储处理数据包过程中发生的错误类型
static char cCrcTmp, cCrcTmp5, cCrcTmp4, cCrcTmp3, cCrcTmp2; // 这些临时变量可能用于CRC计算过程中的中间步骤或其他临时操作
//} RS485_Protocol;

/****************************************************************************
	void Rs485Initialise(void)
	Initialise RS485 network driver 初始化 RS485 网络驱动程序
****************************************************************************/
void Rs485Initialise(char cAddrHigh, char cAddrLow) // 初始化 RS485 网络驱动程序 同时指定将发送数据的从机地址
{
  cOurAddrHigh = cAddrHigh;		// 存储将发送数据的指定从机地址高字节
  cOurAddrLow = cAddrLow;		// 存储将发送数据的指定从机地址低字节
  cRS485State = PKT_WAIT_START; // 初始化状态机指针
  T_RO = 0;
  PIE1bits.RCIE = 1;			// 开启接收中断
}

/****************************************************************************
	char PacketForUs(void)
	Decide if packet valid and destined for this node. 判断数据包是否有效，是否以本节点为目的地。
	Ignore invalid packets and packets for other nodes 忽略无效数据包和其他节点的数据包
****************************************************************************/
char Rs485Process(void) // 数据包效用判断
{
	char cOurPkt, cPktReady;	// 数据包状态
	cOurPkt = FALSE;			// 默认状态
	cPktReady = FALSE;			// 若地址错误 则为假
	INTCONbits.GIE = 0;			// 关闭全局中断
	if(cRS485State == PKT_COMPLETE) // 检查数据包是否轮询完毕
	{
		if((cNetAddrHigh == cOurAddrHigh) && (cNetAddrLow == cOurAddrLow))	// 检查接收的地址位是否对应指定从机的地址位
		{
			cOurPkt = TRUE;		// 若地址正确 则返回真，并执行下一步
		}
		else
		{
			cOurPkt = FALSE;	// 若地址错误 则为假
			cPktReady = FALSE;
			__delay_ms(200);
		}
		cRS485State = PostValidatePacket();			// 验证CRC数据包 确保接收的地址位（数据）是正确的
		if((cRS485State == PKT_INVALID) || (cRS485State == PKT_VALID))	// 在校验完地址位与CRC后判断返回结果是否有效
		{
			// 拒绝无效数据包
			if(cRS485State == PKT_INVALID)			// 拒绝应答无效数据包
			{
				//if(cError == BAD_CRC)	;
				//else if(cError == BAD_LENGTH)	;
				__delay_ms(200);
				if(cOurPkt)	Rs485SendPacket(SENSOR_NAK, 0, NULL);	//	反馈无效数据包
				cRS485State = PKT_WAIT_START;		// 将状态机指针复位 等待下次接收
			}
			else if(cRS485State == PKT_VALID)		// 如果数据包有效
			{										// 并且接收地址正确
				if(cOurPkt) cPktReady = TRUE;		// 地址正确 CRC校验正确 表示已做好解包准备
				else  cRS485State = PKT_WAIT_START;	// 地址错误 将状态机指针复位 等待下次接收
			}
		}
	}
	INTCONbits.GIE = 1;		// 开启全局中断
	return cPktReady;		// 返回一个值 表示是否准备好此次通讯
}

/****************************************************************************
	void Rs485Decode(void)
	Decode an incomming packet on the RS485 network 解码从485接收到的数据包
	Expecting: 字节期望长度
	START, 起始信号
	NETWORK ADDRESS_HIGH, 地址高字节
	NETWORK ADDRESS_LOW, 地址低字节
	PAYLOAD LENGTH, 有效载荷长度
	COMMAND, 控制命令
	optional DATA, 可选数据
	CRC HIGH, CRC高位
	CRC LOW CRC低位
****************************************************************************/
char Rs485Decode(void) // 解码从485接收到的数据包
{
	// 使用状态机来处理接收到的每个字节
	switch (cRS485State) // 根据状态机的当前状态进行解码
	{	// 等待包的开始标志
		case PKT_WAIT_START:	// 0
		  						cStart = cRs485RxChar; // 将接收到的字符存储在 cStart 变量中
								if (cRs485RxChar == PKT_START) // 检查是否为数据包的开始字节
								{
									cRS485State++; // 移动到下一个状态
								}
								break;
		// 等待接收网络地址的高字节
		case PKT_WAIT_ADDR_HIGH:// 1
								cNetAddrHigh = cRs485RxChar; // 存储指定从机发送来的地址
								cRS485State++; // 移动到下一个状态
								break;
		// 等待接收网络地址的低字节
		case PKT_WAIT_ADDR_LOW:	// 2
								cNetAddrLow = cRs485RxChar; // 存储指定从机发送来的地址
								cRS485State++; //移动到下一个状态
								break;
		// 等待接收数据长度字节
		case PKT_WAIT_LEN:		// 3
								cLenExpected = cRs485RxChar; // 存储预期的数据长度
								if (cLenExpected > sizeof(c485Buf)) //检查数据长度是否超过缓冲区的大小
								{
									cRS485State = PKT_INVALID; // 若超过大小 则判断为无效数据包
									cError = BAD_LENGTH; // 返回错误类型为 数据长度错误
								}
								else
								{
									cBufPtr = 0; // 若数据长度小于缓冲区大小 则重置缓冲区指针
									cRS485State++; //移动到下一个状态
								}
								break;
		// 等待接收控制命令字节
		case PKT_CMD:			// 4
								cCommand = cRs485RxChar; // 存储控制命令字节
								if (PacketHasPayload(cCommand)) cRS485State = PKT_WAIT_DATA; // 判断数据包命令类型
								else cRS485State = PKT_WAIT_CRC_HIGH; // 若无法判断则跳过数据 接收CRC校验
								//cRS485State++;
								break;
		// 等待接收数据
		case PKT_WAIT_DATA:		// 5
								c485Buf[cBufPtr] = cRs485RxChar; // 将接收到的数据存储到缓冲区
								cBufPtr++; //移动缓冲区指针
								if (cBufPtr == cLenExpected) // 若接收的数据长度达到预期数据长度
								{
									cRS485State++;	// 移动到下一个状态
								}
								break;
		// 等待接收CRC高字节
		case PKT_WAIT_CRC_HIGH:	// 6
								cRxCrcHigh = cRs485RxChar; // 存储CRC高字节
								cRS485State++; // 移动到下一个字节
								break;
		// 等待接收CRC低字节
		case PKT_WAIT_CRC_LOW:	// 7
								cRxCrcLow = cRs485RxChar; // 存储CRC低字节
								cRS485State = PKT_COMPLETE; // 将状态机设置为已完成接收状态
								break;
		// 数据包已接收完毕 处于空闲状态
		case PKT_COMPLETE:		// 8
								break;       // 不执行任何操作
		case PKT_VALID:			// 9
								break;       // 不执行任何操作
		case PKT_INVALID:		// 255
								break;       // 不执行任何操作
		// 任何其他情况下 重置起始状态
		default:
								cRS485State = PKT_WAIT_START; // 起始状态
								break;
	}
	return cRS485State; // 返回当前的状态机状态
}

/****************************************************************************
	void Rs485SendPacket( char cAddr, char cCmd, char cLen, char *cData )
	Send a packet over the RS485 link 通过RS485链路发送数据包
	Input: NETWORK_ADDRESS, COMMAND, PAYLOAD_LENGTH, optional DATA 输入可选 地址 指令 数据长度 数据
****************************************************************************/
void Rs485SendPacket(char cCmd, char cLen, char *cData) // 通过RS485链路发送数据包
{
	char c, d;
	PIE1bits.RCIE = 0;		// 关闭接收中断
	T_RO = 1;
	__delay_ms(1);			// 缓冲时间
	cCalcCrcHigh = 0xFF;	// 复位CRC
	cCalcCrcLow = 0xFF;
	// Send some NULL preamblesfopr receiving UART 为接收 UART 发送一些 NULL 前置码
	for (c=0; c < NUM_TX_PREAMBLE; c++) Rs485SendChar(0x00);	// 发送3次 NULL 清空发送缓存
	Rs485UpdateCrc(PKT_START);		// 更新待发送的数据CRC
	Rs485SendChar(PKT_START);		// 发送起始信号
	Rs485UpdateCrc(cOurAddrHigh);
	Rs485SendChar(cOurAddrHigh);	// 发送地址高位
	Rs485UpdateCrc(cOurAddrLow);
	Rs485SendChar(cOurAddrLow);		// 发送地址低位
	Rs485UpdateCrc(cLen);
	Rs485SendChar(cLen);			// 发送数据长度
	Rs485UpdateCrc(cCmd);
	Rs485SendChar(cCmd);			// 发送控制命令

	if (cLen != 0)					// 需发送数据长度不得等于0
	{
		for (c = 0; c < cLen; c++)	// 依次发送数据
		{
			d = cData[c];			// 依次提取待发送数据
			Rs485UpdateCrc(d);		// 更新CRC
		}
		for (c = 0; c < cLen; c++)
		{
			d = cData[c];
			Rs485SendChar(d);		// 发送数据
		}
	}
	Rs485SendChar(cCalcCrcHigh);	// 发送CRC校验码高字节
	Rs485SendChar(cCalcCrcLow);		// 发送CRC校验码低字节
	__delay_ms(1);
	T_RO = 0;
	PIE1bits.RCIE = 1;				// Enable Receive Interrupt 打开接收中断
}

/****************************************************************************
	void Rs485GetPacket( char *cCommand, char cLen, char *cData )
	Pass packet to main application 将数据包传递给主程序
****************************************************************************/
void Rs485GetPacket(char *cCom, char *cLen, char *cData) // 将数据包传递给主程序
{
	char c;					// 循环计数
	*cCom = cCommand;		// 将接收到的命令赋值给cCom指针所指向的位置
	*cLen = cLenExpected;	// 将接收到的数据长度赋值给cLen指针所指向的位置
	for (c=0; c < cLenExpected;c++)  cData[c] = c485Buf[c]; // 将接收到的数据赋值给cData指针所指向的位置
	cData[cLenExpected] = 0x00;	// 在数据末尾添加一个空字符null 表示字符串结束
}

/*************************************************************************
	CRC16 Lookup tables (High and Low Byte) for 4 bits per iteration. CRC16 查找表（高字节和低字节），每次迭代 4 位。
*************************************************************************/
const char CRC16_LookupHigh[16] = {
		  0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
		  0x81, 0x91, 0xA1, 0xB1, 0xC1, 0xD1, 0xE1, 0xF1
};	// CRC 查找表
const char CRC16_LookupLow[16] = {
		  0x00, 0x21, 0x42, 0x63, 0x84, 0xA5, 0xC6, 0xE7,
		  0x08, 0x29, 0x4A, 0x6B, 0x8C, 0xAD, 0xCE, 0xEF
};

/****************************************************************************
	Before each message CRC is generated, the CRC register must be 在每次生成报文 CRC 之前，必须通过调用该函数初始化 CRC 寄存器。
	initialised by calling this function 必须通过调用该函数来初始化
****************************************************************************/
void CRC16_Init(void) // CRC初始化
{
	// 根据 CCITT 规范将 CRC 初始化为 0xFFFF
	cCalcCrcHigh = 0xFF;
	cCalcCrcLow = 0xFF;
}

/****************************************************************************
	Process 4 bits of the message to update the CRC Value. 处理报文的 4 位，更新 CRC 值。
	Note that the data must be in the low nibble of val. 注意，数据必须位于 Val 的低位。
****************************************************************************/
void CRC16_Update4Bits(char val) // 更新4位CRC
{
	char t;
	// 第一步，提取 CRC 寄存器中最重要的 4 个比特
	t = cCalcCrcHigh >> 4;	// 将高位寄存器向右移动4位存放进变量里
	// 将信息数据按位异或到提取的比特中
	t = t ^ val;			// 按位异或
	// 将CRC寄存器左移4位
	cCalcCrcHigh = (cCalcCrcHigh << 4) | (cCalcCrcLow >> 4);	// 合并
	cCalcCrcLow = cCalcCrcLow << 4;		// 将低位寄存器向左移动4位
	// 进行查表，并将结果异或到 CRC 表中
	cCalcCrcHigh = cCalcCrcHigh ^ CRC16_LookupHigh[t];	// 与CRC高字节查找表按位异或
	cCalcCrcLow = cCalcCrcLow ^ CRC16_LookupLow[t];		// 与CRC低字节查找表按位异或
}

/****************************************************************************
	Process one Message Byte to update the current CRC Value 处理一个信息字节，更新当前 CRC 值
****************************************************************************/
void Rs485UpdateCrc(char cVal) // 更新CRC
{
	CRC16_Update4Bits(cVal >> 4);	// 处理CRC高字节
	CRC16_Update4Bits(cVal & 0x0F);	// 处理CRC低字节
}

/****************************************************************************
	void Rs485SendChar( char c )
	Driver level of RS485 protocol RS485 协议的驱动器级别 
	Output character on RS485 driver 在 RS485 驱动器上输出字符 
	Include line turn around time 包括线路周转时间
****************************************************************************/
void Rs485SendChar(char c) // 发送一字节
{
	TXREG = c;						// Load data to send 加载数据以发送
	while (!(TXSTA & TRMT_MASK));	// Wait for TX Empty 等待发送完成
}

/****************************************************************************
	char PostValidatePacket(void)
	Verify the CRC on the last packet received 验证最后收到的数据包的 CRC 
	Check if the CRC is correct 检查 CRC 是否正确 
	and return the updated state as the result 并将更新后的状态作为结果返回
****************************************************************************/
char PostValidatePacket(void) // 验证CRC校验码
{
	char c, d;
	CRC16_Init();	// 初始化
	Rs485UpdateCrc(PKT_START);		// 依次更新检查CRC校验码, 起始信号
	Rs485UpdateCrc(cNetAddrHigh);	// 地址高位
	Rs485UpdateCrc(cNetAddrLow);	// 地址低位
	Rs485UpdateCrc(cLenExpected);	// 预期字节长度
	Rs485UpdateCrc(cCommand);		// 控制命令

	// 校验接收的数据
	if (PacketHasPayload(cCommand))			// 如果数据包有有效载荷
	{								// 则在 CRC 中包含数据
		for (c = 0; c < cLenExpected; c++)	// 遍历字节
		{
			d = c485Buf[c];			// 依次读取字节数据
			Rs485UpdateCrc(d);		// 依次进行CRC校验
		}
	}
	// 检查 CRC 是否正确 
	// 并将更新后的状态作为结果返回
	if ( (cRxCrcHigh == cCalcCrcHigh)&&(cRxCrcLow == cCalcCrcLow) ) // 验证校验CRC与接收CRC是否相等
	{
		cRS485State = PKT_VALID;	// 返回有效值
	}
	else
	{
		cError = BAD_CRC;			// 返回校验码错误
		cRS485State = PKT_INVALID;	// 返回无效值
	}
	return cRS485State; // 返回计算后状态机指示器
}

/****************************************************************************
	char GetPacketCmdType(void)
	Check packet command type 检查控制命令类型
	Return TRUE if packet has a data payload. 如果数据包有数据有效载荷，则返回 TRUE
****************************************************************************/
char PacketHasPayload(char ccCommand) // 检查控制命令类型
{
  if (ccCommand == SENSOR_GET_DATA) return TRUE;	// 0xA4 获取数据
  else return FALSE;
}





