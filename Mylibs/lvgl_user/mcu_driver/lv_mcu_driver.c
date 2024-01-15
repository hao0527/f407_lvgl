#include "lv_mcu_driver.h"

void lv_mcu_spiSendData(uint8_t *pData, uint16_t Size)
{
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY);
	HAL_SPI_Transmit(&hspi2, pData, Size, 1000);
}

void lv_mcu_spiSendColor(uint8_t *pData, uint16_t Size)
{
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY);
	HAL_SPI_Transmit_DMA(&hspi2, pData, Size);
}

uint8_t lv_mcu_spiIsReady(void)
{
	uint8_t isReady = HAL_SPI_GetState(&hspi2) == HAL_SPI_STATE_READY;
	return isReady;
}
