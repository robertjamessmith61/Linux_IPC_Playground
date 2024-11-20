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
// <<<<
// >>>> Serial Reader Variables
pthread_t serialReaderThread;
char *
    serialReaderBuf,
    hexString,
    hexPointer,
    charString,
    charPointer;
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
        i,
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

    // Create Serial listener thread.
    result = pthread_create(&serialReaderThread, NULL, SerialReader, (void *)serialFd);
    if (result < 0)
    {
        errnum = errno;
        fprintf(stderr, "Error starting serial listener thread: %s\n", strerror(errnum));
        exit(errnum);
    }

    // INSERT CODE HERE

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

    // free(buf);
    // free(hexString);
    // free(charString);

    // Wait for Serial listener thread to finish.
    pthread_join(serialReaderThread, NULL);

    return 0;
}

static void *SerialReader(void *arg)
{
    // Add our cleanup routine so everything is taken care of if we get cancelled or end gracefully
    pthread_cleanup_push(SerialReaderCleanup, NULL);

    int
        i,
        result;

    fd_set fdSet;

    serialReaderBuf = calloc(BUF_LEN, sizeof(char));
    hexString = calloc(BUF_LEN * 3, sizeof(char));
    charString = calloc(BUF_LEN * 3, sizeof(char));

    while (1)
    {
        FD_SET(serialFd, &fdSet);

        select(serialFd + 1, &fdSet, NULL, NULL, NULL);

        if (FD_ISSET(serialFd, &fdSet))
        {
            result = read(serialFd, serialReaderBuf, BUF_LEN);

            hexPointer = hexString;
            charPointer = charString;

            for (i = 0; i < result; i++)
            {
                hexPointer += sprintf(hexPointer, ",%02X", serialReaderBuf[i]);
            }

            for (i = 0; i < result; i++)
            {
                if (serialReaderBuf[i] >= 32 && serialReaderBuf[i] < 127)
                    charPointer += sprintf(charPointer, ", %c", serialReaderBuf[i]);
                else
                    charPointer += sprintf(charPointer, ",..");
            }

            printf(" %s\n", hexString);
            printf(" %s\n", charString);
        }
    }

    pthread_cleanup_pop(1);
    return NULL;
}

static void SerialReaderCleanup(void *arg)
{
    free(serialReaderBuf);
    free(hexString);
    free(charString);
}