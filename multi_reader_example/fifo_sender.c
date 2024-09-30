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

    const long bufLen = ChunkSize * sizeof(char);

    char *txData = (char *)calloc(ChunkSize, sizeof(char));

    mkfifo(txName, 0666);                        /* read/write for user/group/others */
    int txFd = open(txName, O_CREAT | O_WRONLY); /* open as write-only */
    if (txFd < 0)
        return -1; /** error **/

    while (1)
    {
        printf("Type message and press enter (type exit to close app):\n");
        if (fgets(txData, bufLen, stdin) == NULL)
            continue;

        if (strcmp(txData, "exit\n") == 0)
            break;

        printf("Message:\n");
        printf("%s", txData);

        ssize_t txCount = write(txFd, txData, strcspn(txData, "\0"));
    }

    // close(txFd);    /* close pipe: generates an end-of-file */
    unlink(txName); /* unlink from the implementing file */

    free(txData);

    return 0;
}
