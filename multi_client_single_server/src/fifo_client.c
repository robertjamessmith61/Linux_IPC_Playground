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
static void *PipeListener(void *arg);
static void PipeListenerCleanup(void *arg);
// <<<<
// <<

// TODO figure out why it prints extra newlines when client recieves from server

int main()
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

    listenerReturn = pthread_create(&listenerThread, NULL, PipeListener, (void *)rxName);
    if (listenerReturn != 0)
    {
        fprintf(stderr, "Failed to start listener thread\n");
        return listenerReturn;
    }

    char *senderData;
    senderData = (char *)calloc(CHUNKSIZE, sizeof(char));

    mkfifo(txName, 0666);                        /* read/write for user/group/others */
    int txFd = open(txName, O_CREAT | O_WRONLY); /* open as write-only */
    if (txFd < 0)
    {
        int error = errno;
        fprintf(stderr, "Failed to create named pipe: %s\n", txName);
        free(senderData);
        return error;
    }

    // load up our message to subscribe to the server
    snprintf(senderData, bufLen, "%s,%s", cmdSubscribe, rxName);
    printf("Sending:\n");
    printf("%s\n", senderData);
    // send our subscribe message
    write(txFd, senderData, strcspn(senderData, "\0"));

    // reuse our senderData buffer to read user input looking for the exit command
    while (1)
    {
        printf("Type message and press enter (type exit to close app):\n");
        if (fgets(senderData, bufLen, stdin) == NULL)
            continue;

        if (strcmp(senderData, "exit\n") == 0)
            break;
    }

    int retVal;

    // Tell listenerThread to finish
    retVal = pthread_cancel(listenerThread);
    if (retVal < 0)
    {
        fprintf(stderr, "Failed to send cancelation request to listener thread\n");
    }

    //Wait for listener thread to finish
    pthread_join(listenerThread, NULL);

    // we can close and unlink our own pipe
    close(txFd);
    unlink(txName);
    free(senderData);

    // Just close the server's listener pipe so others can still use it
    close(listenerFd);

    return 0;
}

static void *PipeListener(void *arg)
{
    // Add our cleanup routine so everything is taken care of if we get cancelled or end gracefully
    pthread_cleanup_push(PipeListenerCleanup, NULL);

    // Grab listener pipe name from arg
    listenerName = (char *)arg;

    // Allocate rx buffer
    listenerData = (char *)calloc(CHUNKSIZE, sizeof(char));

    // Create named pipe
    mkfifo(listenerName, 0666);                        // read/write permissions for user/group/everyone
    listenerFd = open(listenerName, O_CREAT | O_RDWR); // Create if doesn't exist, read-only
    if (listenerFd < 0)
    {
        fprintf(stderr, "Failed to create named pipe: %s\n", listenerName);
        pthread_exit(NULL);
    }

    /*
    Listen for incoming connections and process their messages.
    */
    while (1)
    {
        ssize_t count = read(listenerFd, listenerData, bufLen);

        if (count <= 0)
            break; /* end of stream */
        else
            printf("recieved:\n%s\n", listenerData);
    }

    pthread_cleanup_pop(1);
    return NULL;
}

static void PipeListenerCleanup(void *arg)
{
    (void)arg;
    int retVal = 0;
    // Close pipe when we're done
    retVal = close(listenerFd);
    if (retVal < 0)
    {
        fprintf(stderr, "Failed to close file handle: %i\n", listenerFd);
        return;
    }
    unlink(listenerName);

    free(listenerData);

    return;
}