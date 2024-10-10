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
#include <signal.h>
#include "fifo_common.h"
#include "linked_list.h"

// >> Defines
#define MAX_CONNECTIONS 32
// <<

// >> Constants
const long bufLen = CHUNKSIZE * sizeof(char);
const char *rxName = "./fifo_listener";
const char *cmdSubscribe = "subscribe";
// <<

// >> Global variables
// >>>> Listener variables
int listenerFd;
char *listenerData;
char *listenerName;
char *cmdParseData;
// <<<<

// >>>> Sender variables
LinkedList *subscribers;
// <<<<
// <<

// >> Function Declarations
// >>>> Listener pipe function declarations
static void *PipeListener(void *arg);
static void PipeListenerCleanup(void *arg);
static void CleanupSubsciberNode(Node *subscriberNode);
// <<<<
// >>>> Signal handlers
static void HandlerSigpipe(int fd);
// <<<<
// <<

// int main(int argc, char *argv[])
int main()
{
    Node *subscriberNode;

    // Need to intercept SIGPIPE to avoid program ending if a pipe is closed from client end.
    signal(SIGPIPE, HandlerSigpipe);

    // Assign some memory to hold our list of subscribers
    subscribers = (LinkedList *)calloc(1, sizeof(LinkedList));

    // Create our separate thread to handle our listener pipe
    pthread_t listenerThread;
    int listenerReturn;

    listenerReturn = pthread_create(&listenerThread, NULL, PipeListener, (void *)rxName);
    if (listenerReturn != 0)
    {
        fprintf(stderr, "Failed to start listener thread\n");
        free(subscribers);
        return listenerReturn;
    }

    // Allocate a buffer for our data we are going to push to subscribers
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

        subscriberNode = subscribers->first;

        while (subscriberNode != NULL)
        {
            // TODO https://stackoverflow.com/questions/18935446/program-received-signal-sigpipe-broken-pipe

            ssize_t txCount = write(*((int *)subscriberNode->data), senderData, strcspn(senderData, "\0") + 1);

            if (txCount > 0)
            {
                printf("Wrote %li bytes to subscriber %s\n", txCount, (char *)(subscriberNode->key));
            }

            subscriberNode = subscriberNode->next;
        }
    }

    int retVal;

    // Tell listenerThread to finish
    retVal = pthread_cancel(listenerThread);
    if (retVal < 0)
    {
        fprintf(stderr, "Failed to send cancelation request to listener thread\n");
    }

    // Close all of our connections
    subscriberNode = subscribers->first;

    while (subscriberNode != NULL)
    {
        printf("node->%li, key->%li, fd->%li", (long)subscriberNode, (long)subscriberNode->key, (long)subscriberNode->data);

        CleanupSubsciberNode(subscriberNode);
        DelNodeByAddress(subscribers, subscriberNode);
        subscriberNode = subscribers->first;
    }

    // Finally free our memory
    free(subscribers);
    free(senderData);

    // Wait for listenerThread to finish cleaning up
    pthread_join(listenerThread, NULL);

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
    // cmdParseData = (char *)calloc(CHUNKSIZE, sizeof(char));

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

        /*
        Check message type and handle appropriately. Message should be comma separated, starting with a command and
        followed by any arguments
        */
        char *token = strtok(listenerData, ",");
        while (token != NULL)
        {
            printf("Command: %s\n", token);

            // Subscribe command. Add named pipe to subscriber list
            if (strncmp(token, cmdSubscribe, strlen(cmdSubscribe)) == 0)
            {
                token = strtok(NULL, ",");
                printf("pipeName: %s\n", token);

                if (token == NULL)
                {
                    fprintf(stderr, "error: commmand %s missing argument\n", cmdSubscribe);

                    continue; // Start loop again
                }

                size_t pipeNameLen = strlen(token);

                // Check pipe name isn't too long
                if (pipeNameLen + 1 > MAX_PIPE_NAME_LEN)
                {
                    fprintf(stderr, "error: commmand %s - pipe name length is greater than %i: %s\n",
                            cmdSubscribe, MAX_PIPE_NAME_LEN - 1, token);

                    continue; // Start loop again
                }

                // Create new node for our subscriber
                Node *newNode = (Node *)calloc(1, sizeof(Node));

                newNode->key = calloc(pipeNameLen + 1, sizeof(char)); // Add 1 for null terminator
                strncpy(((char *)newNode->key), token, pipeNameLen);

                newNode->data = calloc(1, sizeof(int));

                // Open named pipe
                *((int *)newNode->data) = open(((char *)newNode->key), O_CREAT | O_WRONLY); // Create if doesn't exist, write-only
                if (*((int *)newNode->data) < 0)
                {
                    fprintf(stderr, "Failed to create named pipe: %s\n", ((char *)newNode->key));
                    continue; // Start loop again
                }

                // If we successfully opened the pipe, add it to the subscriber list
                AddNode(subscribers, newNode);
            }

            token = strtok(NULL, ",");
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

    return;
}

static void CleanupSubsciberNode(Node *subscriberNode)
{
    int retVal;

    retVal = close(*((int *)subscriberNode->data));
    if (retVal < 0)
    {
        fprintf(stderr, "Failed to close subscriber file handle: %i\n", *((int *)subscriberNode->data));
    }

    free(subscriberNode->data);
    free(subscriberNode->key);
}

static void HandlerSigpipe(int fd)
{
    fprintf(stderr, "SIGPIPE error, file handle: %i\n", fd);

    int retVal = 0;
    // Close pipe when we're done
    retVal = close(fd);
    if (retVal < 0)
    {
        fprintf(stderr, "Failed to close file handle: %i\n", fd);
    }

    return;
}