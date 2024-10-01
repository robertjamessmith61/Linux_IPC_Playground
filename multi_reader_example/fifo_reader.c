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

    const long bufLen = CHUNKSIZE * sizeof(char);

    char *txData = (char *)calloc(CHUNKSIZE, sizeof(char));

    mkfifo(txName, 0666);                        /* read/write for user/group/others */
    int txFd = open(txName, O_CREAT | O_RDONLY); /* open as read-only */
    // int txFd = open(txName, O_CREAT | O_RDWR); /* open as Read/Write */
    if (txFd < 0)
        return -1; /** error **/

    while (1)
    {
        ssize_t count = read(txFd, txData, bufLen);

        if (count <= 0)
            break; /* end of stream */
        else
            printf("recieved:\n%s", txData);
    }

    close(txFd);    /* close pipe: generates an end-of-file */
    unlink(txName); /* unlink from the implementing file */

    free(txData);

    return 0;
}
