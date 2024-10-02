#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "fifo_common.h"

// >> Constants
const long bufLen = CHUNKSIZE * sizeof(char);
const char *txName = "./fifo_listener";
const char *cmdSubscribe = "subscribe";
// <<

// >> Global variables
// >>>> Listener variables
int listenerFd;
char *listenerData;
char *listenerName;
// <<<<
// <<

// >> Function Declarations
// >>>> Listener pipe function declarations
static int *pipe_listener(void *arg);
static int *pipe_listener_cleanup(void *arg);
// <<<<
// <<

int main(int argc, char *argv[])
{
    char rxName[MAX_PIPE_NAME_LEN];

    int subNameLen = snprintf(rxName, MAX_PIPE_NAME_LEN, "./fifo_sub_%i", getpid());

    if (subNameLen < 0 || subNameLen > MAX_PIPE_NAME_LEN)
    {
        fprintf(stderr, "error: subscriber pipe name length is greater than %i: %s\n",
                MAX_PIPE_NAME_LEN - 1, rxName);
    }

    pthread_t listenerThread;
    int listenerReturn;

    listenerReturn = pthread_create(&listenerThread, NULL, pipe_listener, (void *)rxName);

    char *senderData;
    senderData = (char *)calloc(CHUNKSIZE, sizeof(char));

    mkfifo(txName, 0666);                        /* read/write for user/group/others */
    int txFd = open(txName, O_CREAT | O_WRONLY); /* open as write-only */
    if (txFd < 0)
    {
        int error = errno;
        fprintf(stderr, "Failed to create named pipe: %s\n", txName);
        return error;
    }

    while (1)
    {
        // char txData[CHUNKSIZE];

        ssize_t count = read(txFd, txData, bufLen);

        if (count <= 0)
            break; /* end of stream */
        else
            printf("recieved:\n%s", txData);
    }

    close(txFd);    /* close pipe: generates an end-of-file */
    unlink(txName); /* unlink from the implementing file */

    close(rxFd);
    unlink(listenerName);

    free(txData);
    free(rxData);

    return 0;
}
