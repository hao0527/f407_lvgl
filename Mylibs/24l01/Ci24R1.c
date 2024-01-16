#include "Ci24R1.h"
#include "Ci24R1_spi.h"

#define TX_ADR_WIDTH   4     // 4���ֽڵķ���/���յ�ַ����
#define RX_ADR_WIDTH   4     // 4���ֽڵķ���/���յ�ַ����
#define TX_PLOAD_WIDTH 32    // PLOAD����
#define RX_PLOAD_WIDTH 32    // PLOAD����

// #define BLE_HEAD_LENGTH

// #define USE_IRQ 1

#define MAX_TX  0x10
#define TX_OK   0x20
#define RX_OK   0x40
#define RX_P_NO 0X0E    // ����FIFOΪ�ձ�־


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
��ʼ��Ci24R1��IO��
********************************/
void Ci24R1_Init(void)
{
	Ci24R1_CSN_0;                      // CSN = 0
	Ci24R1_CSN_1;                      // CSN = 1
	// Ci24R1_SCK_0;                      // SCK = 0
	Ci24R1_Write_Reg(SELSPI, 0x00);    // SELSPI--SPI����л�DATA����ΪSPI�ӿ�
	/*ģ�����*/
	Ci24R1_Write_Reg(W_REGISTER + EN_AA, 0x01);        // bit[7:6] 00
	Ci24R1_Write_Reg(W_REGISTER + EN_RXADDR, 0x81);    // bit[7:6] 10    reg0F[3:0]Ϊ 0010 ѡ��0SC���ݿ���
	Ci24R1_Write_Reg(W_REGISTER + 0x0f, 0xa0);         // ����0SC�ڲ�����Ϊ15pf
	Ci24R1_Write_Reg(FLUSH_RX, 0x00);                  // ���TX FIFO��ֵ,��ֹ�ϴη����쳣�˳�
	Ci24R1_Write_Reg(FLUSH_TX, 0x00);                  // ���TX FIFO��ֵ,��ֹ�ϴη����쳣�˳�
	Ci24R1_Write_Reg(W_REGISTER + CONFIG, 0x00);       // bit[7:6] 00
	//		 		/*0F_12*/
	//	Ci24R1_Write_Reg(W_REGISTER+EN_AA,0x01); //bit[7:6] 00
	//	Ci24R1_Write_Reg(W_REGISTER+EN_RXADDR,0xc0); //bit[7:6] 10    reg0F[3:0]Ϊ 0010 ѡ��0SC���ݿ���
	//	Ci24R1_Write_Reg(W_REGISTER+0x0f,0x50);     //����0SC�ڲ�����Ϊ15pf
	//
	//	 	 		/*0f_13*/
	//	Ci24R1_Write_Reg(W_REGISTER+EN_AA,0x41); //bit[7:6] 00
	//	Ci24R1_Write_Reg(W_REGISTER+EN_RXADDR,0xC0); //bit[7:6] 10    reg0F[3:0]Ϊ 0010 ѡ��0SC���ݿ���
	//	Ci24R1_Write_Reg(W_REGISTER+0x0f,0x80);     //����0SC�ڲ�����Ϊ15pf
}

/********************************
���ܣ����Ci24R1��SPIͨ������,ͨ������ַ�Ĵ���д5���ֽ����ݣ�����������֤
********************************/
uint8_t Ci24R1_Check(void)
{
	uint8_t i;
	uint8_t buf[5] = {0xA5, 0xA5, 0xA5, 0xA5, 0xA5};
	HAL_Delay(500);
	Ci24R1_Write_Reg(CE_OFF, 0x00);    // CE�õͣ�����
	Ci24R1_Write_Buf(W_REGISTER + TX_ADDR, (uint8_t *)buf, 5);
	Ci24R1_Read_Buf(R_REGISTER + TX_ADDR, (uint8_t *)buf, 5);
	HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);    // ������ʾ
	for (i = 0; i < 5; i++)
		if (buf[i] != 0xA5)
			break;
	if (i != 5) 
		return 1;    // ���Ci24R1����
	return 0;                // ��⵽Ci24R1
}


/********************************
����Si24R1����һ������
txbuf�������������׵�ַ
����ֵ���������״��
********************************/

uint8_t Ci24R1_TxPacket()
{
	uint8_t status;
	uint8_t user_data[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x01, 0x02, 0x03, 0x04, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x01};

	Ci24R1_Write_Reg(CE_OFF, 0x00);
	Ci24R1_Write_Reg(FLUSH_TX, 0x00);    // ���TX FIFO��ֵ,��ֹ�ϴη����쳣�˳�
	Ci24R1_Write_Buf(W_TX_PAYLOAD_NOACK, user_data, TX_PLOAD_WIDTH);
	Ci24R1_Write_Reg(CE_ON, 0x00);    // CE���ߣ�Ci24R1��ʼ����
	HAL_Delay(10);                    // �ȴ����ݷ������
#ifdef USE_IRQ
	Ci24R1_Write_Reg(SELIRQ, 0x00);
	DATA_in();    // PA6�л�Ϊ�����
	while (Ci24R1_IRQ == 1)
		;                              // һֱ�ȴ�IRQ���ű����ͣ�Ci24R1��IRQ�͵�ƽ��Ч�����ȴ��������
	Ci24R1_Write_Reg(SELSPI, 0x00);    // ��DATA�����л�ΪSPI����
#endif
	status = Ci24R1_Read_Reg(R_REGISTER + STATUS);    // ��ȡ״̬��־
	Ci24R1_Write_Reg(W_REGISTER + STATUS, status);    // ���MAX_TX��TX_OK�жϣ���־λд1�����
	if (status & MAX_TX) {
		Ci24R1_Write_Reg(FLUSH_TX, 0x00);    // ���TX FIFO��ֵ
		return MAX_TX;
	}
	if (status & TX_OK) {
		return TX_OK;
	}
	return 0xff;    // ����ԭ����ʧ��
}

/********************************
����Ci24R1����һ������
rxbuf�������������׵�ַ
����ֵ��0,������ɣ�  1�������������
********************************/
uint8_t Ci24R1_RxPacket()
{
	uint8_t status;

#ifdef USE_IRQ
	Ci24R1_Write_Reg(SELIRQ, 0x00);    // ��DATA�����л�ΪIRQ����
	DATA_in();                         // PA6�л�Ϊ�����
	while (Ci24R1_IRQ == 1)
		;                              // һֱ�ȴ�IRQ���ű����ͣ�Ci24R1��IRQ�͵�ƽ��Ч�����ȴ��������
	Ci24R1_Write_Reg(SELSPI, 0x00);    // ��DATA�����л�ΪSPI����
#endif
	status = Ci24R1_Read_Reg(R_REGISTER + STATUS);
	Ci24R1_Write_Reg(W_REGISTER + STATUS, 0xFF);
	if (status & RX_OK) {
		while ((status & RX_P_NO) != 0x0e)    // �ж�RX_FIFO�Ƿ�Ϊ�գ���Ϊ�ռ�����
		{
			Ci24R1_Read_Buf(R_RX_PAYLOAD, rxbuf, RX_PLOAD_WIDTH);
			status = Ci24R1_Read_Reg(R_REGISTER + STATUS);    // ������״̬�Ĵ����ж�RX_FIFO��־λ
		}
		Ci24R1_Write_Reg(FLUSH_RX, 0x00);    // ���RX_FIFO
		return 0;
	}
	return 1;
}

/********************************
��ʼ��Ci24R1��RXģʽ
����RX��ַ��RX���ݿ�ȣ�ѡ��RFƵ����������
��CE��ߺ󣬼�����RXģʽ�������Խ���������
********************************/
void Ci24R1_RX_Mode(void)
{
	Ci24R1_Write_Reg(CE_OFF, 0x00);
	Ci24R1_Write_Buf(W_REGISTER + RX_ADDR_P0, (uint8_t *)RX_ADDRESS, RX_ADR_WIDTH);    // д��RX�ڵ��ַ
	Ci24R1_Write_Reg(W_REGISTER + SETUP_AW, 0x02);                                     // ���ý��յ�ַ���Ϊ4���ֽڣ�
	Ci24R1_Write_Reg(W_REGISTER + EN_AA, 0X01);                                        // ʹ��ͨ��0���Զ�Ӧ�������Ҫʹ��ShockburstЭ�飬��Ϊ0��
	Ci24R1_Write_Reg(W_REGISTER + EN_RXADDR, 0x01);                                    // ʹ��ͨ��0�Ľ��յ�ַ
	//  Ci24R1_Write_Reg(W_REGISTER + FEATURE, 0x01);
	Ci24R1_Write_Reg(W_REGISTER + RF_CH, 80);                   // ����RFƵ��Ϊ2480MHz
	Ci24R1_Write_Reg(W_REGISTER + RX_PW_P0, RX_PLOAD_WIDTH);    // ѡ��ͨ��0����Ч���ݿ��
	Ci24R1_Write_Reg(W_REGISTER + RF_SETUP, 0x04);              // ����RX���ղ�����11dBm���͹��ʣ�1Mbps�����������濪��
	Ci24R1_Write_Reg(W_REGISTER + CONFIG, 0x0f);                // ���û�������ģʽ�Ĳ�����PWR_UP,EN_CRC,16BIT,����ģʽ
	Ci24R1_Write_Reg(CE_ON, 0x00);                              // CE���ߣ��������ģʽ
}


/**************************************************
��ʼ��Ci24R1��TXģʽ(NOACKģʽ��
����TX��ַ4�ֽڣ�TX���ݿ�ȣ�ѡ��RFƵ��2480MHz�����÷��书�ʺ�����
��CE��ߺ�10us��������������
**************************************************/

void Ci24R1_TX_Mode(void)
{
	Ci24R1_Write_Reg(CE_OFF, 0x00);                                                 // CE���ͣ�оƬ�������ģʽ
	Ci24R1_Write_Reg(W_REGISTER + SETUP_AW, 0x02);                                  // ���ý��յ�ַ���Ϊ4���ֽڣ�
	Ci24R1_Write_Buf(W_REGISTER + TX_ADDR, (uint8_t *)TX_ADDRESS, TX_ADR_WIDTH);    // д��TX�ڵ��ַ
	Ci24R1_Write_Reg(W_REGISTER + EN_AA, 0x01);                                     // ʹ��ͨ��0���Զ�Ӧ�������Ҫʹ��ShockburstЭ�飬��Ϊ0��
	Ci24R1_Write_Reg(W_REGISTER + EN_RXADDR, 0x01);                                 // ʹ��ͨ��0�Ľ��յ�ַ
	Ci24R1_Write_Reg(W_REGISTER + FEATURE, 0x01);                                   //// ʹ��NO_ACK_TX
	// Ci24R1_Write_Reg(W_REGISTER + DYNPD, 0x00);


	Ci24R1_Write_Reg(W_REGISTER + RF_CH, 80);         // ����RFƵ��Ϊ2480MHz
	Ci24R1_Write_Reg(W_REGISTER + RF_SETUP, 0x04);    // ����TX���书��11dbm��1Mbps��������
	Ci24R1_Write_Reg(W_REGISTER + CONFIG, 0x0e);      // TX_DS IRQ enable CRCʹ�ܣ�16λCRCУ�飬�ϵ�, PTX mode
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
