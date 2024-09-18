#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "fifo_common.h"

int main()
{
    printf("Chunksize == %ikB.\n", ChunkSize / 1024);

    const char *txName = "./fifoTx";
    const char *rxName = "./fifoRx";

    const long bufLen = ChunkSize * sizeof(char);

    char *txData = (char *)calloc(ChunkSize, sizeof(char));

    mkfifo(txName, 0666);                        /* read/write for user/group/others */
    int txFd = open(txName, O_CREAT | O_RDONLY); /* open as write-only */
    if (txFd < 0)
        return -1; /** error **/

    mkfifo(rxName, 0666);                        /* read/write for user/group/others */
    int rxFd = open(rxName, O_CREAT | O_WRONLY); /* open as read-only */
    if (rxFd < 0)
        return -1; /** error **/

    while (1)
    {
        // char txData[ChunkSize];

        ssize_t count = read(txFd, txData, bufLen);

        if (count <= 0)
            break; /* end of stream */
        else
            write(rxFd, txData, bufLen);
    }

    close(txFd);    /* close pipe: generates an end-of-file */
    unlink(txName); /* unlink from the implementing file */

    close(rxFd);
    unlink(rxName);

    printf("Done echoing\n");

    free(txData);

    return 0;
}
