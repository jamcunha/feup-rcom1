#include "transmitter.h"

#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "frame_utils.h"

struct {
    int fd;
    struct termios oldtio, newtio;
} transmitter;

int open_transmitter(char* serial_port, int baudrate) {
    transmitter.fd = open(serial_port, O_RDWR | O_NOCTTY);

    if (transmitter.fd < 0) {
        return 1;
    }

    if (tcgetattr(transmitter.fd, &transmitter.oldtio) == -1) {
        return 2;
    }

    memset(&transmitter.newtio, 0, sizeof(transmitter.newtio));

    transmitter.newtio.c_cflag = baudrate | CS8 | CLOCAL | CREAD;
    transmitter.newtio.c_iflag = IGNPAR;
    transmitter.newtio.c_oflag = 0;

    transmitter.newtio.c_lflag = 0;
    transmitter.newtio.c_cc[VTIME] = 0;
    transmitter.newtio.c_cc[VMIN] = 0;

    tcflush(transmitter.fd, TCIOFLUSH);

    if (tcsetattr(transmitter.fd, TCSANOW, &transmitter.newtio) == -1) {
        return 3;
    }

    return 0;
}

int close_transmitter() {
    if (tcsetattr(transmitter.fd, TCSANOW, &transmitter.oldtio) == -1) {
        return 1;
    }

    close(transmitter.fd);
    return 0;
}

int sendSET() {
    uint8_t* frame = build_supervision_frame(TX_ADDRESS, SET_CONTROL);

    if (write(transmitter.fd, frame, 5) != 5) {
        return 1;
    }

    // Wait until all bytes have been written to the serial port
    sleep(1);

    free(frame);
    return 0;
}

int receivedUA() {
    return read_supervision_frame(transmitter.fd, RX_ADDRESS, UA_CONTROL);
}
