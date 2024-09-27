#define baseChunk 1024
#define numBaseChunks 8
#define ChunkSize baseChunk *numBaseChunks /* how many written at a time */

struct fifoConnection
{
    int connectionActive;
    int listenerFd;
    char *listenerData;
    char *listenerName;
};