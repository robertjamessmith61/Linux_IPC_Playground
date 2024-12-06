#ifndef COBS_ENCODING_H
#define COBS_ENCODING_H

#include <stdint.h>

int CobsEncode(uint8_t *dataIn, int dataInCount, uint8_t *encodedOut);
int CobsDecode(uint8_t *encodedIn, int encodedInCount, uint8_t *dataOut);

#endif /* #define COBS_ENCODING_H */