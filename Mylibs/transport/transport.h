#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "main.h"

#define TRANSPORT_FRAME_SIZE 32                                      // һ֡�����ֽ���
#define FRAME_SAMPLE_TIMES   ((TRANSPORT_FRAME_SIZE - 3) * 2 + 1)    // һ֡�ܴ��Ĳ�������

void transport_encodeAudioFrame(const int16_t *inbuf, uint8_t *outbuf, uint8_t rc);
void transport_decodeAudioFrame(uint8_t *inbuf, int16_t *outbuf, uint8_t *rc);
void transport_test(void);

void transport_sendAudio(const int16_t *inbuf);
void transport_recvAudio(uint8_t *inbuf);

#endif /*TRANSPORT_H*/
