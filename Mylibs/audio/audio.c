#include <string.h>
#include "audio.h"

AudioInfo_t audioInfo;

static inline void checkAdcContinue(void)
{
	if (!audioInfo.inDmaBusy && audioInfo.inFlag[audioInfo.inBufferIndex] == 0) {
		audioInfo.inDmaBusy = 1;
		HAL_ADC_Start_DMA(&hadc1, (void *)audioInfo.inPcm[audioInfo.inBufferIndex], FRAME_SAMPLE_TIMES);
	}
}

static inline void checkDacContinue(void)
{
	if (!audioInfo.outDmaBusy && audioInfo.outFlag[audioInfo.outBufferIndex] == 1) {
		audioInfo.outDmaBusy = 1;
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (void *)audioInfo.outPcm[audioInfo.outBufferIndex], FRAME_SAMPLE_TIMES, DAC_ALIGN_12B_R);
	}
}

static inline void checkEncodeContinue(void)
{
	uint8_t i, idx;
	for (i = 0; i < AUDIO_BUFFER_NUM; i++) {
		idx = (audioInfo.inBufferIndex + 1 + i) % AUDIO_BUFFER_NUM;
		if (audioInfo.inFlag[idx] == 1) {
			transport_sendAudio(audioInfo.inPcm[idx]);
			audioInfo.inFlag[idx] = 0;
			break;
		}
	}
}

void audio_DecodeReady(int16_t *pcm)
{
	uint8_t i, idx;
	for (i = 0; i < AUDIO_BUFFER_NUM; i++) {
		idx = (audioInfo.outBufferIndex + 1 + i) % AUDIO_BUFFER_NUM;
		if (audioInfo.outFlag[idx] == 0) {
			memcpy(audioInfo.outPcm[idx], pcm, 2*FRAME_SAMPLE_TIMES);
			audioInfo.outFlag[idx] = 1;
			break;
		}
	}
}

void audio_init(void)
{
#if AUDIO_IN_EN == 1
	HAL_TIM_Base_Start(&htim3);
	// HAL_ADC_Start_DMA(&hadc1, (void *)audioInfo.inPcm[audioInfo.inBufferIndex], FRAME_SAMPLE_TIMES);
#endif
#if AUDIO_OUT_EN == 1
	HAL_TIM_OC_Start(&htim5, TIM_CHANNEL_1);
	// HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (void *)audioInfo.outPcm[audioInfo.outBufferIndex], FRAME_SAMPLE_TIMES, DAC_ALIGN_12B_R);
#endif
}

void audio_main(void)
{
	checkAdcContinue();
	checkDacContinue();
	checkEncodeContinue();
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	audioInfo.inFlag[audioInfo.inBufferIndex] = 1;
	if (++audioInfo.inBufferIndex == AUDIO_BUFFER_NUM)
		audioInfo.inBufferIndex = 0;
	audioInfo.inDmaBusy = 0;
	checkAdcContinue();
	audioInfo.inCnt += FRAME_SAMPLE_TIMES;
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
	static __IO uint32_t error;
	(void)error;
	error = HAL_ADC_GetError(hadc);
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
	audioInfo.outFlag[audioInfo.outBufferIndex] = 0;
	if (++audioInfo.outBufferIndex == AUDIO_BUFFER_NUM)
		audioInfo.outBufferIndex = 0;
	audioInfo.outDmaBusy = 0;
	checkDacContinue();
	audioInfo.outCnt += FRAME_SAMPLE_TIMES;
}

void HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef *hdac)
{
	static __IO uint32_t error;
	(void)error;
	error = HAL_DAC_GetError(hdac);
}

void HAL_DAC_DMAUnderrunCallbackCh1(DAC_HandleTypeDef *hdac)
{
	static __IO uint32_t error;
	(void)error;
	error = HAL_DAC_GetError(hdac);
	HAL_DAC_Start_DMA(hdac, DAC_CHANNEL_1, (void *)audioInfo.outPcm[audioInfo.outBufferIndex], FRAME_SAMPLE_TIMES, DAC_ALIGN_12B_R);
}
