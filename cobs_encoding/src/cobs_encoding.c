
// https://blog.mbedded.ninja/programming/serialization-formats/consistent-overhead-byte-stuffing-cobs/

/*
Encodes the first dataInCount bytes from dataIn with the Consistent Overhead Byte Stuffing framing.
Writes the resultant encoded output bytes to encodedOut.
Returns the number of bytes written to encodedOut.
*/
int CobsEncode(char *encodedOut, char *dataIn, int dataInCount)
{
    int
        i,
        zeroPointer = 0,
        zeroCounter = 1,
        outWritePtr = 1;

    for (i = 0; i < dataInCount; i++)
    {
        if (dataIn[i] == 0)
        {
            encodedOut[zeroPointer] = zeroCounter;
            zeroCounter = 1;
            zeroPointer = outWritePtr++;
        }
        else
        {
            encodedOut[outWritePtr++] = dataIn[i];
            zeroCounter++;
        }
    }
    encodedOut[zeroPointer] = zeroCounter;
    encodedOut[outWritePtr++] = 0;

    return outWritePtr;
}

/*
Decodes the first encodedInCount bytes from encodedIn with the Consistent Overhead Byte Stuffing framing.
Writes the resultant decoded output bytes to dataOut.
Returns the number of bytes written to dataOut.
*/
int CobsDecode(char *dataOut, char *encodedIn, int encodedInCount)
{
    int
        i,
        zeroPointer = encodedIn[0],
        outWritePtr = 0;

    for (i = 1; i < encodedInCount; i++)
    {
        if (i == zeroPointer)
        {
            zeroPointer = i + encodedIn[i];
            if (encodedIn[i] == 0)
                break;
            dataOut[outWritePtr++] = 0;
        }
        else
        {
            dataOut[outWritePtr++] = encodedIn[i];
        }
    }

    return outWritePtr;
}