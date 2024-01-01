#include "Ci24R1_spi.h"

/*************************************************************************************************
函数名称：Ci24R1_SPI_WriteByte
功能：根据SPI协议，写一字节数据到Ci24R1
参数：src，待写入值
返回值：
***************************************************************************************************/
void Ci24R1_SPI_WriteByte(uint8_t src)
{
	HAL_SPI_Transmit(&hspi3, &src, 1, 1000);
}

/*************************************************************************************************
函数名称：Ci24R1_SPI_ReadByte
功能：根据SPI协议，从Ci24R1读出一字节
参数：
返回值：RxData 读到的字节数据
***************************************************************************************************/
uint8_t Ci24R1_SPI_ReadByte(void)
{
	uint8_t rxData, txData = 0xff;
	// HAL_SPI_Receive(&hspi3, &rxData, 1, 1000);
	HAL_SPI_TransmitReceive(&hspi3, &txData, &rxData, 1, 1000);
	return rxData;
}

/*DATA IO口初始化为输出模式*/
void DATA_out(void)
{
}

/*DATA IO口初始化为输入模式*/
void DATA_in(void)
{
}

/********************************************
功能：SPI写寄存器
参数：reg:指定寄存器的地址
			value：要写入的值
********************************************/
void Ci24R1_Write_Reg(uint8_t reg, uint8_t value)
{
	Ci24R1_CSN_0;                   // CSN置低，开始传输数据
	Ci24R1_SPI_WriteByte(reg);      // 选择寄存器
	Ci24R1_SPI_WriteByte(value);    // 然后写数据到该寄存器
	Ci24R1_CSN_1;                   // CSN拉高，结束数据传输
}

/********************************************
功能：从reg寄存器读一个字节的数据
参数：reg:指定寄存器的地址
********************************************/
uint8_t Ci24R1_Read_Reg(uint8_t reg)
{
	uint8_t reg_val;
	Ci24R1_CSN_0;                       // CSN置低，开始传输数据
	Ci24R1_SPI_WriteByte(reg);          // 选择寄存器
	reg_val = Ci24R1_SPI_ReadByte();    // 然后从该寄存器读数据
	Ci24R1_CSN_1;                       // CSN拉高，结束数据传输
	return reg_val;                     // 返回寄存器数据
}

/*******************************************************************************
功能：把pBuf缓存中的数据写入到Ci24R1，通常用来写入发射通道数据 或 接收/发送地址
参数：reg:指定寄存器的地址
*pBuf:数据指针
length:数据长度
*******************************************************************************/
void Ci24R1_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t length)
{
	uint8_t i;
	Ci24R1_CSN_0;                 // CSN置低，开始传输数据
	Ci24R1_SPI_WriteByte(reg);    // 选择寄存器
	for (i = 0; i < length; i++) {
		Ci24R1_SPI_WriteByte(*pBuf++);    // 逐个字节写入Ci24R1
	}
	Ci24R1_CSN_1;    // CSN拉高，结束数据传输
}

/*************************************************************************
功能：从reg寄存器读出length个字节，通常用来读取接收通道数据 或 接收/发送地址
参数：reg:指定寄存器的地址
*pBuf:数据指针
length:数据长度
*************************************************************************/
void Ci24R1_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t length)
{
	uint8_t i;
	Ci24R1_CSN_0;                 // CSN置低，开始传输数据
	Ci24R1_SPI_WriteByte(reg);    // 选择寄存器
	for (i = 0; i < length; i++) {
		pBuf[i] = Ci24R1_SPI_ReadByte();    // 逐个字节从Ci24R1读出
	}
	Ci24R1_CSN_1;    // CSN拉高，结束数据传输
}
