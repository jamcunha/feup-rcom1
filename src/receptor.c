#include "receptor.h"

#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "common.h"

struct {
    int fd;
    struct termios oldtio, newtio;
} receptor;

int open_receptor(char* serial_port, int baudrate) {
    receptor.fd = open(serial_port, O_RDWR | O_NOCTTY);
    if (receptor.fd < 0) {
        return 1;
    }

    if (tcgetattr(receptor.fd, &receptor.oldtio) == -1) {
        return 2;
    }

    memset(&receptor.newtio, 0, sizeof(receptor.newtio));

    receptor.newtio.c_cflag = baudrate | CS8 | CLOCAL | CREAD;
    receptor.newtio.c_iflag = IGNPAR;
    receptor.newtio.c_oflag = 0;

    receptor.newtio.c_lflag = 0;
    receptor.newtio.c_cc[VTIME] = 0;
    receptor.newtio.c_cc[VMIN] = 0;

    tcflush(receptor.fd, TCIOFLUSH);

    if (tcsetattr(receptor.fd, TCSANOW, &receptor.newtio) == -1) {
        return 3;
    }

    return 0;
}

int close_receptor() {
    if (tcsetattr(receptor.fd, TCSANOW, &receptor.oldtio) == -1) {
        return 1;
    }

    close(receptor.fd);
    return 0;
}

int connect_receptor() {
    // ask teacher if receiver is supposed to wait forever
    while (read_supervision_frame(receptor.fd, TX_ADDRESS, SET_CONTROL) != 0) {}

    if (send_supervision_frame(receptor.fd, RX_ADDRESS, UA_CONTROL)) {
        return 1;
    }

    return 0;
}
