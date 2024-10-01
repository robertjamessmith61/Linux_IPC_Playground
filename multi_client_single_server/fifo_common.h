#define BASE_CHUNK 1024
#define NUM_BASE_CHUNKS 4
#define CHUNKSIZE BASE_CHUNK *NUM_BASE_CHUNKS /* how many written at a time */

#define MAX_PIPE_NAME_LEN 64 /* limit pipe names to a low number to avoid any possible issues */

/*
Note Linux should have a PIPE_BUF value of 4096. Meaning that writing more bytes than this at once may be split into
multiple packets and can be interleaved with other processes writing to the same pipe.
*/

struct fifoConnection
{
    int connectionActive;
    int listenerFd;
    char *listenerData;
    char *listenerName;
};