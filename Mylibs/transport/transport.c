#include <stdlib.h>
#include "transport.h"
#include "adpcm-lib.h"
#include "audio.h"

/**
 * @brief 编码一帧音频
 * @param inbuf pcm存放的地址
 * @param outbuf 发送数据的地址
 * @param rc 5bits rollcnt
 */
void transport_encodeAudioFrame(const int16_t *inbuf, uint8_t *outbuf, uint8_t rc)
{
	void *adpcm_cnxt = NULL;
	size_t outSize;
	int32_t deltas[2] = {abs(inbuf[1] - inbuf[0])};
	adpcm_cnxt = adpcm_create_context(1, 0, NOISE_SHAPING_STATIC, deltas);
	adpcm_encode_block(adpcm_cnxt, outbuf, &outSize, inbuf, FRAME_SAMPLE_TIMES);
	adpcm_free_context(adpcm_cnxt);
	outbuf[1] &= 0x0f;
	outbuf[2] &= 0x7f;
	outbuf[1] |= rc << 4;
	outbuf[2] |= rc << 3 & 0x80;
}

/**
 * @brief 解码一帧音频
 * @param inbuf 接收数据的地址
 * @param pcmData pcm存放的地址
 * @param rc 5bits rollcnt
 */
void transport_decodeAudioFrame(uint8_t *inbuf, int16_t *outbuf, uint8_t *rc)
{
	*rc = 0;
	*rc = inbuf[1] >> 4 | (inbuf[2] & 0x80) >> 3;
	inbuf[1] &= 0x0f;
	inbuf[2] &= 0x7f;
	adpcm_decode_block(outbuf, inbuf, TRANSPORT_FRAME_SIZE, 1);
}

#include "pcmIn.h"
#define PCM_FRAME_NUM (sizeof(pcmIn) / FRAME_SAMPLE_TIMES / 2)
int16_t pcmOut[PCM_FRAME_NUM][FRAME_SAMPLE_TIMES];
uint8_t adpcmData[PCM_FRAME_NUM][32];
void transport_test(void)
{
	uint8_t rc;
	for (int i = 0; i < PCM_FRAME_NUM; i++) {
		transport_encodeAudioFrame(pcmIn[i], adpcmData[i], (uint8_t)i);    // 0.1443ms @168M
	}

	for (int i = 0; i < PCM_FRAME_NUM; i++) {
		transport_decodeAudioFrame(adpcmData[i], pcmOut[i], &rc);    // 0.0308ms @168M
	}
}

void transport_sendAudio(const int16_t *inbuf)
{
	static uint8_t rc;
	static uint8_t outbuf[TRANSPORT_FRAME_SIZE];
	transport_encodeAudioFrame(inbuf, outbuf, rc++);
	transport_recvAudio(outbuf);
}

void transport_recvAudio(uint8_t *inbuf)
{
	static uint8_t rc;
	static int16_t outbuf[FRAME_SAMPLE_TIMES];
	transport_decodeAudioFrame(inbuf, outbuf, &rc);
	audio_DecodeReady(outbuf);
}
