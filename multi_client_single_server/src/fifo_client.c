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
volatile __sig_atomic_t listenerComplete;
// <<<<
// >>>> Console variables
char *consoleData;
// <<<<
// <<

// >> Function Declarations
// >>>> Listener pipe function declarations
static void *PipeListener(void *arg);
static void PipeListenerCleanup(void *arg);
// <<<<
// >>>> Console IO function declarations
static void *ConsoleIO(void *arg);
static void ConsoleIOCleanup(void *arg);
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
    pthread_t consoleThread;
    int threadReturn;

    listenerComplete = 0;

    threadReturn = pthread_create(&listenerThread, NULL, PipeListener, (void *)rxName);
    if (threadReturn != 0)
    {
        fprintf(stderr, "Failed to start listener thread\n");
        return threadReturn;
    }

    threadReturn = pthread_create(&consoleThread, NULL, ConsoleIO, NULL);
    if (threadReturn != 0)
    {
        fprintf(stderr, "Failed to start console IO thread\n");
        return threadReturn;
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
    snprintf(senderData, bufLen, ",%s,%s,", cmdSubscribe, rxName);
    printf("Sending:\n");
    printf("%s\n", senderData);
    // send our subscribe message
    write(txFd, senderData, strcspn(senderData, "\0"));

    free(senderData);

    // Just close the server's listener pipe so others can still use it
    close(txFd);

    while (!listenerComplete)
    {
        sleep(1);
    }

    int retVal;

    // Tell listenerThread to finish
    retVal = pthread_cancel(listenerThread);
    if (retVal < 0)
    {
        fprintf(stderr, "Failed to send cancelation request to listener thread\n");
    }

    // Wait for listener thread to finish
    pthread_join(listenerThread, NULL);

    // Tell consoleThread to finish
    retVal = pthread_cancel(consoleThread);
    if (retVal < 0)
    {
        fprintf(stderr, "Failed to send cancelation request to console IO thread\n");
    }

    // Wait for console thread to finish
    pthread_join(consoleThread, NULL);

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
    mkfifo(listenerName, 0666);                          // read/write permissions for user/group/everyone
    listenerFd = open(listenerName, O_CREAT | O_RDONLY); // Create if doesn't exist, read-only
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
        {
            // listenerData[count] = 0; // null terminate string
            printf("recieved:\n%s", listenerData);
        }
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

    listenerComplete = 1;
}

static void *ConsoleIO(void *arg)
{
    (void)arg;

    // Add our cleanup routine so everything is taken care of if we get cancelled or end gracefully
    pthread_cleanup_push(ConsoleIOCleanup, NULL);

    consoleData = (char *)calloc(CHUNKSIZE, sizeof(char));

    printf("Type exit to close app:\n");
    do
    {
        fgets(consoleData, bufLen, stdin);
    } while (strcmp(consoleData, "exit\n") != 0 && listenerComplete == 0);    

    pthread_cleanup_pop(1);

    return NULL;
}

static void ConsoleIOCleanup(void *arg)
{
    (void)arg;

    free(consoleData);

    listenerComplete = 1;
}