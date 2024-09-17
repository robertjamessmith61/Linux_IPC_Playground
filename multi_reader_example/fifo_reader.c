#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fifo_common.h"

int main(int argc, char *argv[])
{
    const char *txName = "./fifo_multi_Tx";
    const char *rxName = "./fifo_multi_Rx";

    const long bufLen = ChunkSize * sizeof(char);

    char *txData = (char *)calloc(ChunkSize, sizeof(char));
    char *rxData = (char *)calloc(ChunkSize, sizeof(char));

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
            printf("recieved:\n%s", txData);
    }

    close(txFd);    /* close pipe: generates an end-of-file */
    unlink(txName); /* unlink from the implementing file */

    close(rxFd);
    unlink(rxName);

    free(txData);
    free(rxData);

    return 0;
}
