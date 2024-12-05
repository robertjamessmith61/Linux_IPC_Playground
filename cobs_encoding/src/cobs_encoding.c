#include <stdio.h>
#include <stdlib.h>

int CobsEncode(unsigned char *dataIn, int dataInCount, unsigned char *encodedOut);
int CobsDecode(unsigned char *encodedIn, int encodedInCount, unsigned char *dataOut);

int main() {
    int
        i,
        myArrayLen = 300;
        
    unsigned char
        *myArray = calloc(4096, sizeof(unsigned char)),
        *encArray = calloc(4096, sizeof(unsigned char)),
        *decArray = calloc(4096, sizeof(unsigned char));
    
    for (i = 0; i < myArrayLen; i++)
        myArray[i] = rand() % 255;
    
    // for (i = 0; i < myArrayLen; i++)
    //     printf(" | %02i, %02x", i, myArray[i]);
    
    // printf("\n");
    
    printf("\nEncode\n");
    int encArrayLen = CobsEncode(myArray, myArrayLen, encArray);
    
    printf("\nreturned %i\n", encArrayLen);
    
    for (i = 0; i < encArrayLen; i++)
    {
        printf(" | %04i, %02x", i, encArray[i]);
        if (i % 8 == 0)
            printf("\n");
    }
        
    printf("\n");
    
    // printf("OG array\n");
    
    // for (i = 0; i < myArrayLen; i++)
    //     printf(" | %02i, %02x", i, myArray[i]);
    
    // printf("\n");
    
    printf("\nDecode\n");
    int decArrayLen = CobsDecode(encArray, encArrayLen, decArray);
        
    printf("\nreturned %i\n", decArrayLen);
    
    // for (i = 0; i < decArrayLen; i++)
    //     printf(" | %02i, %02x", i, decArray[i]);
        
    // printf("\n");
        
    // printf("OG array\n");
    
    // for (i = 0; i < myArrayLen; i++)
    //     printf(" | %02i, %02x", i, myArray[i]);
    
    // printf("\n");

    return 0;
}

int CobsEncode(unsigned char *dataIn, int dataInCount, unsigned char *encodedOut)
{
    int
        i,
        zeroPointer,
        zeroCounter,
        outWritePtr;

    zeroPointer = 0;
    zeroCounter = 1;
    outWritePtr = 1;
    for (i = 0; i < dataInCount; i++)
    {
        // printf("dataIn[%02i] == %02x\n", i, dataIn[i]);
        if (dataIn[i] != 0)
        {
            encodedOut[outWritePtr] = dataIn[i];
            outWritePtr++;
            zeroCounter++;
        }
        if (dataIn[i] == 0 || zeroCounter == 0xff)
        {
            printf("| %02i ", zeroCounter);
            encodedOut[zeroPointer] = zeroCounter;
            zeroCounter = 1;
            zeroPointer = outWritePtr;
            outWritePtr++;
        }
        
        // int j;
        // for (j = 0; j < outWritePtr; j++)
        //     printf(" | %02i, %02x", j, encodedOut[j]);
        // printf("\n");
    }
    encodedOut[zeroPointer] = zeroCounter;
    encodedOut[outWritePtr++] = 0;
    
    printf("\n");

    return outWritePtr;
}

int CobsDecode(unsigned char *encodedIn, int encodedInCount, unsigned char *dataOut)
{
    int
        i,
        zeroPointer,
        outWritePtr;

    zeroPointer = encodedIn[0];
    outWritePtr = 0;
    for (i = 1; i < encodedInCount; i++)
    {
        // printf("zeroPointer == %02i\n", zeroPointer);
        // printf("encodedIn[%02i] == %02x\n", i, encodedIn[i]);
        if (i == zeroPointer)
        {
            zeroPointer = i + encodedIn[i];
            printf("new zeroPointer == %02i\n", zeroPointer);
            if (encodedIn[i] == 0)
                break;
            if (encodedIn[i] != 255)
                dataOut[outWritePtr++] = 0;
        }
        else
        {
            dataOut[outWritePtr++] = encodedIn[i];
        }
        
        // int j;
        // for (j = 0; j < outWritePtr; j++)
        //     printf(" | %02i, %02x", j, dataOut[j]);
        // printf("\n");
    }

    return outWritePtr;
}