#include "cobs_encoding.h"

int CobsEncode(uint8_t *dataIn, int dataInCount, uint8_t *encodedOut)
{
    int
        i,
        zeroPointer,
        outWritePtr;
    
    uint8_t zeroCounter;

    zeroPointer = 0;
    zeroCounter = 1;
    outWritePtr = 1;
    for (i = 0; i < dataInCount; i++)
    {
        if (dataIn[i] != 0)
        {
            encodedOut[outWritePtr++] = dataIn[i];
            zeroCounter++;
        }
        if (dataIn[i] == 0 || zeroCounter == 0xff)
        {
            encodedOut[zeroPointer] = zeroCounter;
            zeroCounter = 1;
            zeroPointer = outWritePtr++;
        }
    }
    encodedOut[zeroPointer] = zeroCounter;
    encodedOut[outWritePtr++] = 0;

    return outWritePtr;
}

int CobsDecode(uint8_t *encodedIn, int encodedInCount, uint8_t *dataOut)
{
    int
        i,
        zeroPointer,
        outWritePtr;
        
    uint8_t nextZeroPtr;

    zeroPointer = 0;
    nextZeroPtr = encodedIn[0];
    outWritePtr = 0;
    for (i = 1; i < encodedInCount; i++)
    {
        if (i == zeroPointer + nextZeroPtr)
        {
            if (encodedIn[i] == 0)
                break;
            if (nextZeroPtr != 255)
            {
                dataOut[outWritePtr++] = 0;
            }
            
            zeroPointer = i;
            nextZeroPtr = encodedIn[i];
        }
        else
        {
            dataOut[outWritePtr++] = encodedIn[i];
        }
    }

    return outWritePtr;
}