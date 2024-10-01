#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "fifo_common.h"

void printUsage();
void printBytes(long bytes, char *retString);

int main(int argc, char *argv[])
{
    int MaxLoops;

    if (argc != 2)
    {
        printf("Incorrect number of arguments [%i]\n", argc - 1);
        printUsage();
        exit(-1);
    }
    else
    {
        char *endptr;
        MaxLoops = strtol(argv[1], &endptr, 10);
        if (MaxLoops == 0)
        {
            printf("[num_loops] must be a non zero positive integer.\n");
            printUsage();
            exit(-1);
        }
    }

    printf("CHUNKSIZE == %ikB.\n", CHUNKSIZE / 1024);

    const char *txName = "./fifoTx";
    const char *rxName = "./fifoRx";

    const long bufLen = CHUNKSIZE * sizeof(char);

    char *txData = (char *)calloc(CHUNKSIZE, sizeof(char));
    char *rxData = (char *)calloc(CHUNKSIZE, sizeof(char));

    mkfifo(txName, 0666);                        /* read/write for user/group/others */
    int txFd = open(txName, O_CREAT | O_WRONLY); /* open as write-only */
    if (txFd < 0)
        return -1; /** error **/

    mkfifo(rxName, 0666);                        /* read/write for user/group/others */
    int rxFd = open(rxName, O_CREAT | O_RDONLY); /* open as read-only */
    if (rxFd < 0)
        return -1; /** error **/

    long bytesRead = 0;
    long bytesSent = 0;

    clock_t begin = clock();

    int i;
    for (i = 0; i < MaxLoops; i++)
    { /* write MaxWrites times */
        // char txData[CHUNKSIZE];

        long j;
        for (j = 0; j < (long)CHUNKSIZE; j++)
        {
            txData[j] = (char)rand();
        }

        ssize_t txCount = write(txFd, txData, bufLen);

        if (txCount <= 0)
        {
            break; /* end of stream */
        }
        else
            bytesSent += (long)txCount;

        ssize_t rxCount = read(rxFd, rxData, bufLen);

        if (rxCount <= 0)
        {
            break; /* end of stream */
        }
        else
            bytesRead += (long)rxCount;
    }

    clock_t end = clock();

    close(txFd);    /* close pipe: generates an end-of-file */
    unlink(txName); /* unlink from the implementing file */

    close(rxFd);
    unlink(rxName);

    double time_spent = (double)(end - begin);

    printf("\n");

    char sByteLen[64];

    printBytes(bytesRead, sByteLen);
    printf("%s bytes recieved from the pipe.\n", sByteLen);

    printBytes(bytesSent, sByteLen);
    printf("%s bytes sent to the pipe.\n", sByteLen);

    printf("\n");

    printf("%fms time elapsed.\n", time_spent / 1000);                             // milliseconds
    printf("%fus roundtrip latency per chunk.\n", time_spent / MaxLoops);          // microseconds
    printf("%fns roundtrip latency per byte.\n", (time_spent / bytesRead) * 1000); // nanoseconds

    free(txData);
    free(rxData);

    return 0;
}

void printUsage()
{
    printf("Tests named pipe latency by sending random bytes.\n");
    printf("Usage: fifosend [num_loops]\n");
}

void printBytes(long bytes, char *retString)
{
    char *notation[] = {"B", "kB", "MB", "GB", "TB"};

    int notationIndex = 0;

    int i;
    for (i = 0; i < sizeof(notation); i++)
    {
        if (bytes > 1024)
        {
            bytes /= 1024;
            notationIndex++;
        }
    }

    sprintf(retString, "%li%s", bytes, notation[notationIndex]);
}
