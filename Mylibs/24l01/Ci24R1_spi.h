#ifndef _Ci24R1_SPI_H
#define _Ci24R1_SPI_H

#include "main.h"

#define Ci24R1_CSN_0  HAL_GPIO_WritePin(NRF_CS_GPIO_Port, NRF_CS_Pin, GPIO_PIN_RESET);    // SPI片选信号
#define Ci24R1_CSN_1  HAL_GPIO_WritePin(NRF_CS_GPIO_Port, NRF_CS_Pin, GPIO_PIN_SET);      // SPI片选信号
// #define Ci24R1_SCK_0  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
// #define Ci24R1_SCK_1  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
// #define Ci24R1_IRQ    HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6)
// #define Ci24R1_DATA_0 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
// #define Ci24R1_DATA_1 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
// #define Ci24R1_DATA   HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6)

void Ci24R1_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t length);
void Ci24R1_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t length);
void Ci24R1_Write_Reg(uint8_t reg, uint8_t value);
uint8_t Ci24R1_Read_Reg(uint8_t reg);
void DATA_out(void);
void DATA_in(void);

#endif
