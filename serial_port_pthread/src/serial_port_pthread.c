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
int serialFd;
// <<

// >> Function Declarations
static void *SerialReader(void * arg);
static void SerialReaderCleanup(void *arg);
// <<

int main()
{
    int
        i,
        errnum,
        result;

    fd_set fdSet;

    struct termios
        oldtio,
        newtio;

    char
        *buf = calloc(BUF_LEN, sizeof(char)),
        *hexString = calloc(BUF_LEN * 3, sizeof(char)),
        *hexPointer,
        *charString = calloc(BUF_LEN * 3, sizeof(char)),
        *charPointer;

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


    tcflush(serialFd, TCIFLUSH);
    tcsetattr(serialFd, TCSANOW, &newtio);

    while (1)
    {
        FD_SET(serialFd, &fdSet);

        select(serialFd + 1, &fdSet, NULL, NULL, NULL);

        if (FD_ISSET(serialFd, &fdSet))
        {
            result = read(serialFd, buf, BUF_LEN);

            hexPointer = hexString;
            charPointer = charString;

            for (i = 0; i < result; i++)
            {
                hexPointer += sprintf(hexPointer, ",%02X", buf[i]);
            }

            for (i = 0; i < result; i++)
            {
                if (buf[i] >= 32 && buf[i] < 127)
                    charPointer += sprintf(charPointer, ", %c", buf[i]);
                else
                    charPointer += sprintf(charPointer, ",..");
            }

            printf(" %s\n", hexString);
            printf(" %s\n", charString);
        }
    }
    // Important, we restore original serial device config before exiting
    tcsetattr(serialFd, TCSANOW, &oldtio);

    free(buf);
    free(hexString);
    free(charString);
    return 0;
}


static void *SerialReader(void *arg)
{
    
}