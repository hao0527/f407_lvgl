#include "Ci24R1.h"
#include "Ci24R1_spi.h"

#define TX_ADR_WIDTH   4     // 4个字节的发送/接收地址长度
#define RX_ADR_WIDTH   4     // 4个字节的发送/接收地址长度
#define TX_PLOAD_WIDTH 32    // PLOAD长度
#define RX_PLOAD_WIDTH 32    // PLOAD长度

// #define BLE_HEAD_LENGTH

// #define USE_IRQ 1

#define MAX_TX  0x10
#define TX_OK   0x20
#define RX_OK   0x40
#define RX_P_NO 0X0E    // 接收FIFO为空标志


uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0xE7, 0xE7, 0xE7, 0xE7};
uint8_t RX_ADDRESS[RX_ADR_WIDTH] = {0xE7, 0xE7, 0xE7, 0xE7};
uint8_t TX_BUF[TX_PLOAD_WIDTH];
uint8_t TX_REG18_2M[4] = {0x82, 0x79, 0x80, 0x00};
uint8_t TX_REG18_1M[4] = {0x82, 0x71, 0x82, 0x00};
uint8_t TX_REG18_250k[4] = {0x82, 0x71, 0x82, 0x00};
uint8_t TX_REG18[4] = {0};
uint8_t rxbuf[TX_PLOAD_WIDTH];
extern uint8_t user_data[];
extern uint8_t rf_tx_buffer[];

uint8_t aa;
/********************************
初始化Ci24R1的IO口
********************************/
void Ci24R1_Init(void)
{
	Ci24R1_CSN_0;                      // CSN = 0
	Ci24R1_CSN_1;                      // CSN = 1
	// Ci24R1_SCK_0;                      // SCK = 0
	Ci24R1_Write_Reg(SELSPI, 0x00);    // SELSPI--SPI命令，切换DATA引脚为SPI接口
	/*模拟电容*/
	Ci24R1_Write_Reg(W_REGISTER + EN_AA, 0x01);        // bit[7:6] 00
	Ci24R1_Write_Reg(W_REGISTER + EN_RXADDR, 0x81);    // bit[7:6] 10    reg0F[3:0]为 0010 选择0SC电容控制
	Ci24R1_Write_Reg(W_REGISTER + 0x0f, 0xa0);         // 配置0SC内部电容为15pf
	Ci24R1_Write_Reg(FLUSH_RX, 0x00);                  // 清除TX FIFO的值,防止上次发射异常退出
	Ci24R1_Write_Reg(FLUSH_TX, 0x00);                  // 清除TX FIFO的值,防止上次发射异常退出
	Ci24R1_Write_Reg(W_REGISTER + CONFIG, 0x00);       // bit[7:6] 00
	//		 		/*0F_12*/
	//	Ci24R1_Write_Reg(W_REGISTER+EN_AA,0x01); //bit[7:6] 00
	//	Ci24R1_Write_Reg(W_REGISTER+EN_RXADDR,0xc0); //bit[7:6] 10    reg0F[3:0]为 0010 选择0SC电容控制
	//	Ci24R1_Write_Reg(W_REGISTER+0x0f,0x50);     //配置0SC内部电容为15pf
	//
	//	 	 		/*0f_13*/
	//	Ci24R1_Write_Reg(W_REGISTER+EN_AA,0x41); //bit[7:6] 00
	//	Ci24R1_Write_Reg(W_REGISTER+EN_RXADDR,0xC0); //bit[7:6] 10    reg0F[3:0]为 0010 选择0SC电容控制
	//	Ci24R1_Write_Reg(W_REGISTER+0x0f,0x80);     //配置0SC内部电容为15pf
}

/********************************
功能：检测Ci24R1的SPI通信正常,通过往地址寄存器写5个字节数据，读出来后验证
********************************/
uint8_t Ci24R1_Check(void)
{
	uint8_t i;
	uint8_t buf[5] = {0xA5, 0xA5, 0xA5, 0xA5, 0xA5};
	HAL_Delay(500);
	Ci24R1_Write_Reg(CE_OFF, 0x00);    // CE置低，待机
	Ci24R1_Write_Buf(W_REGISTER + TX_ADDR, (uint8_t *)buf, 5);
	Ci24R1_Read_Buf(R_REGISTER + TX_ADDR, (uint8_t *)buf, 5);
	HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);    // 闪灯提示
	for (i = 0; i < 5; i++)
		if (buf[i] != 0xA5)
			break;
	if (i != 5) 
		return 1;    // 检测Ci24R1错误
	return 0;                // 检测到Ci24R1
}


/********************************
启动Si24R1发送一次数据
txbuf：待发送数据首地址
返回值：发送完成状况
********************************/

uint8_t Ci24R1_TxPacket()
{
	uint8_t status;
	uint8_t user_data[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x01, 0x02, 0x03, 0x04, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x01};

	Ci24R1_Write_Reg(CE_OFF, 0x00);
	Ci24R1_Write_Reg(FLUSH_TX, 0x00);    // 清除TX FIFO的值,防止上次发射异常退出
	Ci24R1_Write_Buf(W_TX_PAYLOAD_NOACK, user_data, TX_PLOAD_WIDTH);
	Ci24R1_Write_Reg(CE_ON, 0x00);    // CE拉高，Ci24R1开始发射
	HAL_Delay(10);                    // 等待数据发送完成
#ifdef USE_IRQ
	Ci24R1_Write_Reg(SELIRQ, 0x00);
	DATA_in();    // PA6切换为输入口
	while (Ci24R1_IRQ == 1)
		;                              // 一直等待IRQ引脚被拉低（Ci24R1的IRQ低电平有效），等待发射完成
	Ci24R1_Write_Reg(SELSPI, 0x00);    // 将DATA引脚切换为SPI功能
#endif
	status = Ci24R1_Read_Reg(R_REGISTER + STATUS);    // 读取状态标志
	Ci24R1_Write_Reg(W_REGISTER + STATUS, status);    // 清除MAX_TX或TX_OK中断（标志位写1清除）
	if (status & MAX_TX) {
		Ci24R1_Write_Reg(FLUSH_TX, 0x00);    // 清除TX FIFO的值
		return MAX_TX;
	}
	if (status & TX_OK) {
		return TX_OK;
	}
	return 0xff;    // 其他原因发送失败
}

/********************************
启动Ci24R1接收一次数据
rxbuf：待接收数据首地址
返回值：0,接收完成；  1：其他错误代码
********************************/
uint8_t Ci24R1_RxPacket()
{
	uint8_t status;

#ifdef USE_IRQ
	Ci24R1_Write_Reg(SELIRQ, 0x00);    // 将DATA引脚切换为IRQ功能
	DATA_in();                         // PA6切换为输入口
	while (Ci24R1_IRQ == 1)
		;                              // 一直等待IRQ引脚被拉低（Ci24R1的IRQ低电平有效），等待发射完成
	Ci24R1_Write_Reg(SELSPI, 0x00);    // 将DATA引脚切换为SPI功能
#endif
	status = Ci24R1_Read_Reg(R_REGISTER + STATUS);
	Ci24R1_Write_Reg(W_REGISTER + STATUS, 0xFF);
	if (status & RX_OK) {
		while ((status & RX_P_NO) != 0x0e)    // 判断RX_FIFO是否为空，不为空继续读
		{
			Ci24R1_Read_Buf(R_RX_PAYLOAD, rxbuf, RX_PLOAD_WIDTH);
			status = Ci24R1_Read_Reg(R_REGISTER + STATUS);    // 继续读状态寄存器判断RX_FIFO标志位
		}
		Ci24R1_Write_Reg(FLUSH_RX, 0x00);    // 清空RX_FIFO
		return 0;
	}
	return 1;
}

/********************************
初始化Ci24R1到RX模式
设置RX地址，RX数据宽度，选择RF频道，波特率
当CE变高后，即进入RX模式，并可以接收数据了
********************************/
void Ci24R1_RX_Mode(void)
{
	Ci24R1_Write_Reg(CE_OFF, 0x00);
	Ci24R1_Write_Buf(W_REGISTER + RX_ADDR_P0, (uint8_t *)RX_ADDRESS, RX_ADR_WIDTH);    // 写入RX节点地址
	Ci24R1_Write_Reg(W_REGISTER + SETUP_AW, 0x02);                                     // 设置接收地址宽度为4个字节；
	Ci24R1_Write_Reg(W_REGISTER + EN_AA, 0X01);                                        // 使能通道0的自动应答（如果需要使用Shockburst协议，设为0）
	Ci24R1_Write_Reg(W_REGISTER + EN_RXADDR, 0x01);                                    // 使能通道0的接收地址
	//  Ci24R1_Write_Reg(W_REGISTER + FEATURE, 0x01);
	Ci24R1_Write_Reg(W_REGISTER + RF_CH, 80);                   // 设置RF频率为2480MHz
	Ci24R1_Write_Reg(W_REGISTER + RX_PW_P0, RX_PLOAD_WIDTH);    // 选择通道0的有效数据宽度
	Ci24R1_Write_Reg(W_REGISTER + RF_SETUP, 0x04);              // 设置RX接收参数，11dBm发送功率，1Mbps，低噪声增益开启
	Ci24R1_Write_Reg(W_REGISTER + CONFIG, 0x0f);                // 配置基本工作模式的参数，PWR_UP,EN_CRC,16BIT,接收模式
	Ci24R1_Write_Reg(CE_ON, 0x00);                              // CE拉高，进入接收模式
}


/**************************************************
初始化Ci24R1到TX模式(NOACK模式）
设置TX地址4字节，TX数据宽度，选择RF频道2480MHz，设置发射功率和速率
当CE变高后，10us后启动发射数据
**************************************************/

void Ci24R1_TX_Mode(void)
{
	Ci24R1_Write_Reg(CE_OFF, 0x00);                                                 // CE拉低，芯片进入待机模式
	Ci24R1_Write_Reg(W_REGISTER + SETUP_AW, 0x02);                                  // 设置接收地址宽度为4个字节；
	Ci24R1_Write_Buf(W_REGISTER + TX_ADDR, (uint8_t *)TX_ADDRESS, TX_ADR_WIDTH);    // 写入TX节点地址
	Ci24R1_Write_Reg(W_REGISTER + EN_AA, 0x01);                                     // 使能通道0的自动应答（如果需要使用Shockburst协议，设为0）
	Ci24R1_Write_Reg(W_REGISTER + EN_RXADDR, 0x01);                                 // 使能通道0的接收地址
	Ci24R1_Write_Reg(W_REGISTER + FEATURE, 0x01);                                   //// 使能NO_ACK_TX
	// Ci24R1_Write_Reg(W_REGISTER + DYNPD, 0x00);


	Ci24R1_Write_Reg(W_REGISTER + RF_CH, 80);         // 设置RF频率为2480MHz
	Ci24R1_Write_Reg(W_REGISTER + RF_SETUP, 0x04);    // 设置TX发射功率11dbm，1Mbps发射速率
	Ci24R1_Write_Reg(W_REGISTER + CONFIG, 0x0e);      // TX_DS IRQ enable CRC使能，16位CRC校验，上电, PTX mode
}

void Ci24R1_Reg18_change()
{
	// uint8_t i=0;
	Ci24R1_Write_Reg(CE_OFF, 0xff);
	Ci24R1_Write_Reg(0x50, 0xa5);
	// Ci24R1_Write_Reg(CE_ON,0xff);

	Ci24R1_Read_Buf(W_REGISTER + BandWidth_TX, (uint8_t *)TX_REG18, 4);
	Ci24R1_Write_Buf(W_REGISTER + BandWidth_TX, (uint8_t *)TX_REG18_2M, 4);
	Ci24R1_Write_Reg(CE_OFF, 0xff);
	Ci24R1_Write_Reg(0x50, 0xa5);
}
