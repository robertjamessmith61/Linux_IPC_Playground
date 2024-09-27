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
#include "linked_list.h"

// https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html

// -- Defines
#define MAX_CONNECTIONS 32
// --

// -- Constants
const long bufLen = ChunkSize * sizeof(char);
// --

// -- Global variables
// ---- Listener variables
int listenerFd;
char *listenerData;
const char *listenerName;
// ----
// ---- Sender variables
LinkedList *subscibers;
// --

// -- Function Declarations
// ---- Listener pipe function declarations
static int *pipe_listener(void *arg);
static int *pipe_listener_cleanup(void *arg);
// ----
// --

int main(int argc, char *argv[])
{
    const char *rxName = "./fifo_listener";

    // Assign some memory to hold our list of subscribers
    subscibers = (LinkedList *)calloc(1, sizeof(LinkedList));

    pthread_t listenerThread;
    int listenerReturn;

    listenerReturn = pthread_create(&listenerThread, NULL, pipe_listener, (void *)rxName);

    char *senderData;
    senderData = (char *)calloc(ChunkSize, sizeof(char));

    while (1)
    {
        printf("Type message and press enter (type exit to close app):\n");
        if (fgets(senderData, bufLen, stdin) == NULL)
            continue;

        if (strcmp(senderData, "exit\n") == 0)
            break;

        printf("Message:\n");
        printf("%s", senderData);

        // ssize_t txCount = write(txFd, senderData, strcspn(senderData, "\0"));
    }

    pthread_join(listenerThread, NULL);

    return 0;
}

static int *pipe_listener(void *arg)
{
    // Add our cleanup routine so everything is taken care of if we get cancelled or end gracefully
    pthread_cleanup_push(pipe_listener_cleanup, NULL);

    // Grab listener pipe name from arg
    listenerName = (char *)arg;

    // Allocate rx buffer
    listenerData = (char *)calloc(ChunkSize, sizeof(char));

    // Create named pipe
    mkfifo(listenerName, 0666);                          // read/write permissions for user/group/everyone
    listenerFd = open(listenerName, O_CREAT | O_RDONLY); // Create if doesn't exist, read-only
    if (listenerFd < 0)
    {
        int error = errno;
        printf(stderr, "Failed to create named pipe: %s", listenerName);
        pthread_exit(error);
    }

    while (1)
    {
        ssize_t count = read(listenerFd, listenerData, bufLen);

        if (count <= 0)
            break; /* end of stream */
        else
            printf("recieved:\n%s", listenerData);
    }

    pthread_cleanup_pop(1);
    return 0;
}

static int *pipe_listener_cleanup(void *arg)
{
    int retVal = 0;
    // Close pipe when we're done
    retVal = close(listenerFd);
    if (retVal < 0)
    {
        int error = errno;
        printf(stderr, "Failed to close file handle: %i", listenerFd);
        return error;
    }
    unlink(listenerName);

    free(listenerData);

    return 0;
}