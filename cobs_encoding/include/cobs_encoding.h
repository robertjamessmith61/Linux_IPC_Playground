#ifndef COBS_ENCODING_H
#define COBS_ENCODING_H

int CobsEncode(char *encodedOut, char *dataIn, int dataInCount);
int CobsDecode(char *dataOut, char *encodedIn, int encodedInCount);

#endif /* #define COBS_ENCODING_H */