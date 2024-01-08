#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "main.h"

void transport_encodeAudioFrame(const int16_t *inbuf, uint8_t *outbuf, uint8_t rc);
void transport_decodeAudioFrame(uint8_t *inbuf, int16_t *outbuf, uint8_t *rc);
void transport_test(void);

#endif /*TRANSPORT_H*/
