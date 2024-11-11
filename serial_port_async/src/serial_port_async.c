#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

// https://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html
// https://stackoverflow.com/questions/76897228/linux-uart-read-in-non-blocking-mode-is-stuck-while-other-low-priority-thread-is
// https://www.man7.org/linux/man-pages/man3/termios.3.html
// https://linux.die.net/man/7/aio
// https://stackoverflow.com/questions/39289676/linux-serial-communication-with-interrupts


// >> Defines
#define BAUDRATE 115200
#define BUF_LEN 4095 // Max input buffer length according to termios man page
#define SERIAL_DEV "/dev/ttyUSB1"
// <<

// >> Variables

// <<

int main()
{
    int fd;

    struct termios
        oldtio,
        newtio;

    char buf[BUF_LEN];

    // Try to open serial device
    fd = open(SERIAL_DEV, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        int errnum = errno;
        fprintf(stderr, "Error opening serial device: %s\n", strerror(errnum));
        exit(errnum);
    }

    // Store our original serial device config
    tcgetattr(fd, &oldtio);

    // Zero new serial device config
    memset(&newtio, 0, sizeof(newtio));
    // Setup our new serial device config
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL |CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0; // RAW mode

    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 10;

    return 0;
}