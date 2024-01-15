#ifndef AUDIO_H
#define AUDIO_H

#include "main.h"
#include "transport.h"

#define AUDIO_BUFFER_NUM 10

#define AUDIO_IN_EN  (1)
#define AUDIO_OUT_EN (1)

typedef struct AudioInfo {
#if AUDIO_IN_EN == 1
	int16_t inPcm[AUDIO_BUFFER_NUM][FRAME_SAMPLE_TIMES];
    uint8_t inFlag[AUDIO_BUFFER_NUM];   // dma搬完置1 编码完置0
	uint8_t inBufferIndex;
    uint8_t inDmaBusy;
    uint32_t inCnt;
#endif
#if AUDIO_OUT_EN == 1
	int16_t outPcm[AUDIO_BUFFER_NUM][FRAME_SAMPLE_TIMES];
    uint8_t outFlag[AUDIO_BUFFER_NUM];   // 解码完置1 dma搬完置0 
	uint8_t outBufferIndex;
    uint8_t outDmaBusy;
    uint32_t outCnt;
#endif
} AudioInfo_t;

extern AudioInfo_t audioInfo;

void audio_init(void);
void audio_main(void);
void audio_DecodeReady(int16_t *pcm);

#endif /*AUDIO_H*/
