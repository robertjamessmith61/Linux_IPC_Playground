#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "fifo_common.h"

// https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html

void pipe_listener(void *arg);

int main(int argc, char *argv[])
{
    const char *rxName = "./fifo_multi_Rx";

    const long bufLen = ChunkSize * sizeof(char);

    char *rxData = (char *)calloc(ChunkSize, sizeof(char));

    mkfifo(rxName, 0666);                        /* read/write for user/group/others */
    int rxFd = open(rxName, O_CREAT | O_RDONLY); /* open as read-only */
    if (rxFd < 0)
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

    close(rxFd);
    unlink(rxName);

    free(rxData);

    return 0;
}
