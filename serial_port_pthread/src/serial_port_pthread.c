#define _XOPEN_SOURCE 700

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

// https://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html
// https://stackoverflow.com/questions/76897228/linux-uart-read-in-non-blocking-mode-is-stuck-while-other-low-priority-thread-is
// https://www.man7.org/linux/man-pages/man3/termios.3.html
// https://linux.die.net/man/7/aio
// https://stackoverflow.com/questions/39289676/linux-serial-communication-with-interrupts

// >> Defines
#define BAUDRATE B115200
#define BUF_LEN 4095 // Max input buffer length according to termios man page
#define SERIAL_DEV "/dev/ttyUSB0"
// <<

// >> Global Variables
// >>>> Global Serial Variables
int serialFd;
struct termios
    oldtio,
    newtio;

int serialDebugFd[2];
// <<<<
// >>>> Serial Reader Variables
pthread_t serialReaderThread;
char
    *serialReaderBuf,
    *hexString,
    *hexPointer,
    *charString,
    *charPointer;
// <<<<
// <<

// >> Function Declarations
// static void MainCleanup(void * arg);
// >>>> Serial Reader
static void *SerialReader(void *arg);
static void SerialReaderCleanup(void *arg);
// <<<<
// <<

int main()
{
    int
        errnum,
        result;

    // >> SERIAL SETUP START

    // Try to open serial device
    serialFd = open(SERIAL_DEV, O_RDWR | O_NOCTTY);
    if (serialFd < 0)
    {
        errnum = errno;
        fprintf(stderr, "Error opening serial device [%s]: %s\n", SERIAL_DEV, strerror(errnum));
        exit(errnum);
    }

    // Store our original serial device config
    tcgetattr(serialFd, &oldtio);

    // Zero new serial device config
    memset(&newtio, 0, sizeof(newtio));
    // Setup our new serial device config
    // newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_cflag = CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0; // RAW mode

    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;

    // Manually set baud rate in case cflag above doesn't work.
    cfsetospeed(&newtio, B115200);
    cfsetispeed(&newtio, 0); // set to zero to match output speed above.

    // Flush buffer and write new settings to serial device.
    tcflush(serialFd, TCIFLUSH);
    tcsetattr(serialFd, TCSANOW, &newtio);

    // << SERIAL SETUP FINISH

    // Create simple pipe for passing debug messages out of serialReaderThread
    result = pipe(serialDebugFd);
    if (result < 0)
    {
        errnum = errno;
        fprintf(stderr, "Error creating serialDebugFd pipe: %s\n", strerror(errnum));
        exit(errnum);
    }

    // Create Serial listener thread.
    result = pthread_create(&serialReaderThread, NULL, SerialReader, NULL);
    if (result < 0)
    {
        errnum = errno;
        fprintf(stderr, "Error starting serial listener thread: %s\n", strerror(errnum));
        exit(errnum);
    }

    fd_set fdSetMain;
    char *inputData = calloc(BUF_LEN, sizeof(char));
    char *debugData = calloc(BUF_LEN * 3, sizeof(char));
    while (1)
    {
        printf("Type exit to close app:\n");

        FD_SET(0, &fdSetMain);
        FD_SET(serialDebugFd[0], &fdSetMain);
        select(serialDebugFd[0] + 1, &fdSetMain, NULL, NULL, NULL);

        if (FD_ISSET(0, &fdSetMain))
        {
            if (fgets(inputData, BUF_LEN, stdin) == NULL)
                continue;

            if (strcmp(inputData, "exit\n") == 0)
                break;
        }
        else if (FD_ISSET(serialDebugFd[0], &fdSetMain))
        {
            // Print out debug message
            result = read(serialDebugFd[0], debugData, BUF_LEN * 3);
            if (result < 0)
            {
                errnum = errno;
                fprintf(stderr, "Error reading from serialDebugPipe: %s\n", strerror(errnum));
            }
            else
            {
                int bytesWritten = 0;
                // char *debugDataPointer = debugData;

                while (bytesWritten < result - 1)
                {
                    bytesWritten += printf("%s\n", debugData + bytesWritten);
                }
            }
        }
    }
    free(inputData);

    // Close our end of debug pipe now we're done
    close(serialDebugFd[0]);

    // Request Serial listener thread to finish.
    result = pthread_cancel(serialReaderThread);
    if (result < 0)
    {
        errnum = errno;
        fprintf(stderr, "Failed to send cancellation listener thread: %s\n", strerror(errnum));
        exit(errnum);
    }

    // Try to close serial device
    result = close(serialFd);
    if (result < 0)
    {
        errnum = errno;
        fprintf(stderr, "Error closing serial device [%s]: %s\n", SERIAL_DEV, strerror(errnum));
        exit(errnum);
    }

    // Important, we restore original serial device config before exiting
    tcsetattr(serialFd, TCSANOW, &oldtio);

    // Wait for Serial listener thread to finish.
    pthread_join(serialReaderThread, NULL);

    return 0;
}

static void *SerialReader(void *arg)
{
    (void)arg;

    // Add our cleanup routine so everything is taken care of if we get cancelled or end gracefully
    pthread_cleanup_push(SerialReaderCleanup, arg);

    int
        i,
        result;

    serialReaderBuf = calloc(BUF_LEN, sizeof(char));
    hexString = calloc(BUF_LEN * 3, sizeof(char));
    charString = calloc(BUF_LEN * 3, sizeof(char));

    write(serialDebugFd[1], "serialReaderThread Started...", 30);

    while (1)
    {
        result = read(serialFd, serialReaderBuf, BUF_LEN);

        hexPointer = hexString;
        charPointer = charString;

        hexPointer += sprintf(hexPointer, "HEX STRING ");
        for (i = 0; i < result; i++)
        {
            hexPointer += sprintf(hexPointer, ",%02X", serialReaderBuf[i]);
        }

        charPointer += sprintf(charPointer, "CHAR STRING ");
        for (i = 0; i < result; i++)
        {
            if (serialReaderBuf[i] >= 32 && serialReaderBuf[i] < 127)
                charPointer += sprintf(charPointer, ", %c", serialReaderBuf[i]);
            else
                charPointer += sprintf(charPointer, ",..");
        }

        write(serialDebugFd[1], charString, strlen(charString) + 1);
        write(serialDebugFd[1], hexString, strlen(hexString) + 1);

    }

    pthread_cleanup_pop(1);
    return NULL;
}

static void SerialReaderCleanup(void *arg)
{
    (void)arg;
    close(serialDebugFd[1]);

    free(serialReaderBuf);
    free(hexString);
    free(charString);
}