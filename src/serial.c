
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/errno.h>

#include "tinyppp6.h"


#define BAUD_RATE   B115200


int serial_open(const char *device_path)
{
    struct termios tios;
    int fd;

    fprintf(stderr, "Opening serial port: %s\n", device_path);

    fd = open(device_path, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        perror(device_path);
        exit(EXIT_FAILURE);
    }

    // Turn back on blocking reads
    fcntl(fd, F_SETFL, 0);

    // Read existing serial port settings
    tcgetattr(fd, &tios);

    // Set the input and output baud rates
    cfsetispeed(&tios, BAUD_RATE);
    cfsetospeed(&tios, BAUD_RATE);

    // Set to local mode
    tios.c_cflag |= CLOCAL | CREAD;

    // Set serial port mode to 8N1
    tios.c_cflag &= ~PARENB;
    tios.c_cflag &= ~CSTOPB;
    tios.c_cflag &= ~CSIZE;
    tios.c_cflag |= CS8;

    // Turn off flow control and ignore parity
    tios.c_cflag &= ~CRTSCTS;
    tios.c_iflag = IGNBRK | IGNPAR;

    // Turn off output post-processing
    tios.c_oflag = 0;

    // set input mode (non-canonical, no echo,...)
    //tios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tios.c_lflag = 0;

    tios.c_cc[VMIN]     = 1;
    tios.c_cc[VTIME]    = 0;

    while (tcsetattr(fd, TCSAFLUSH, &tios) < 0)
        if (errno != EINTR)
            fprintf(stderr, "tcsetattr: %d", errno);

    // Give device time to reset and then flush the input buffer
    sleep(1);
    tcflush(fd, TCIOFLUSH);

    fcntl(fd, F_SETFL, 0);

    return fd;
}
