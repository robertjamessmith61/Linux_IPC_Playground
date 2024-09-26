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

// -- Constants
const long bufLen = ChunkSize * sizeof(char);
// --

// -- Global variables
// ---- Listener variables
int listenerFd;
char *listenerData;
const char *listenerName;
// ----
// --

// -- Function Declarations
// ---- Listener pipe function declarations
static void *pipe_listener(void *arg);
static void *pipe_listener_cleanup(void *arg);
// ----
// --

int main(int argc, char *argv[])
{
    const char *rxName = "./fifo_listener";

    pthread_t listenerThread;
    int listenerReturn;

    listenerReturn = pthread_create(&listenerThread, NULL, pipe_listener, (void *)rxName);

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

    pthread_join(listenerThread, NULL);

    return 0;
}

static void *pipe_listener(void *arg)
{
    // Add our cleanup routine so everything is taken care of if we get cancelled or end gracefully
    pthread_cleanup_push(pipe_listener_cleanup, NULL);

    // Grab listener pipe name from arg
    listenerName = (char *)arg;

    // Allocate rx buffer
    listenerData = (char *)calloc(ChunkSize, sizeof(char));

    // Create named pipe
    mkfifo(listenerName, 0666);                          /* read/write for user/group/others */
    listenerFd = open(listenerName, O_CREAT | O_RDONLY); /* open as read-only */
    if (listenerFd < 0)
        return -1; /** error **/

    while (1)
    {
        ssize_t count = read(listenerFd, listenerData, bufLen);

        if (count <= 0)
            break; /* end of stream */
        else
            printf("recieved:\n%s", listenerData);
    }

    return 0;
}

static void *pipe_listener_cleanup(void *arg)
{
    // Close pipe when we're done
    close(listenerFd);
    unlink(listenerName);

    free(listenerData);
}