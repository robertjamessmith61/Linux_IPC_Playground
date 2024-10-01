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
#include "lib/linked_list/linked_list.h"

// https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html

// -- Defines
#define MAX_CONNECTIONS 32
// --

// -- Constants
const long bufLen = CHUNKSIZE * sizeof(char);
const char *cmdSubscribe = "subscribe";
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
    senderData = (char *)calloc(CHUNKSIZE, sizeof(char));

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

        Node *subscriberNode = subscibers->first;

        while (subscriberNode != NULL)
        {
            ssize_t txCount = write((*(int *)subscriberNode->data), senderData, strcspn(senderData, "\0"));

            if (txCount > 0)
            {
                printf("Wrote %i bytes to subscriber %s", txCount, (char *)(subscriberNode->key));
            }
        }
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
    listenerData = (char *)calloc(CHUNKSIZE, sizeof(char));

    // Create named pipe
    mkfifo(listenerName, 0666);                          // read/write permissions for user/group/everyone
    listenerFd = open(listenerName, O_CREAT | O_RDWR); // Create if doesn't exist, read-only
    if (listenerFd < 0)
    {
        int error = errno;
        fprintf(stderr, "Failed to create named pipe: %s\n", listenerName);
        pthread_exit(error);
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

        /*
        Check message type and handle appropriately. Message should be comma separated, starting with a command and
        followed by any arguments
        */
        char *token = strtok(listenerData, ",");
        while (token != NULL)
        {
            printf("Command: %s\n", token);

            // subscribe command. Add named pipe to subscriber list
            if (strncmp(token, cmdSubscribe, strlen(cmdSubscribe)) == 0)
            {
                token = strtok(NULL, ",");

                if (token == NULL)
                {
                    fprintf(stderr, "error: commmand %s missing argument\n", cmdSubscribe);

                    continue; // start loop again
                }

                size_t pipeNameLen = strlen(token);

                // check pipe name isn't too long
                if (pipeNameLen > MAX_PIPE_NAME_LEN)
                {
                    fprintf(stderr, "error: commmand %s pipe name length is greater than %i: %s\n",
                            cmdSubscribe, MAX_PIPE_NAME_LEN, token);

                    continue; // start loop again
                }

                // Create new node for our subscriber
                Node *newNode = (Node *)calloc(1, sizeof(Node));

                newNode->key = calloc(pipeNameLen + 1, sizeof(char)); // add 1 for null terminator
                strncpy(((char *)newNode->key), token, pipeNameLen);

                newNode->data = calloc(1, sizeof(int));

                // open named pipe
                *((int *)newNode->data) = open(((char *)newNode->key), O_CREAT | O_WRONLY); // Create if doesn't exist, write-only
                if (*((int *)newNode->data) < 0)
                {
                    fprintf(stderr, "Failed to create named pipe: %s\n", ((char *)newNode->key));
                    continue; // start loop again
                }

                // if we successfully opened the pipe, add it to the subscriber list
                AddNode(subscibers, newNode);
            }
        }
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
        fprintf(stderr, "Failed to close file handle: %i\n", listenerFd);
        return error;
    }
    unlink(listenerName);

    free(listenerData);

    return 0;
}