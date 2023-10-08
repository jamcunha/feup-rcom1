#include "transmitter.h"

#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "common.h"

struct {
    int fd;
    struct termios oldtio, newtio;
} transmitter;

alarm_t alarm_config;

void handle_set(int signo) {
    alarm_config.count++;
    send_supervision_frame(transmitter.fd, TX_ADDRESS, SET_CONTROL);
    alarm(alarm_config.timeout);
}

void handle_disc(int signo) {
    alarm_config.count++;
    send_supervision_frame(transmitter.fd, TX_ADDRESS, DISC_CONTROL);
    alarm(alarm_config.timeout);
}

int open_transmitter(char* serial_port, int baudrate, int timeout, int nRetransmissions) {
    alarm_config.count = 0;
    alarm_config.timeout = timeout;
    alarm_config.num_retransmissions = nRetransmissions;
    alarm_config.handler = NULL;

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

int connect_trasmitter() {
    alarm_config.handler = handle_set;

    (void)signal(SIGALRM, alarm_config.handler);
    alarm_config.count = 0;

    if (send_supervision_frame(transmitter.fd, TX_ADDRESS, SET_CONTROL)) {
        return 1;
    }
    alarm(alarm_config.timeout);

    int flag = 0;
    for (;;) {
        if (read_supervision_frame(transmitter.fd, RX_ADDRESS, UA_CONTROL) == 0) {
            flag = 1;
            break;
        }

        // Ask teacher if is 1st try + 3 alarm tries or 3 tries as a whole
        if (alarm_config.count == alarm_config.num_retransmissions) {
            break;
        }
    }
    alarm(0);

    if (!flag) {
        return 2;
    }
    
    return 0;
}

int disconnect_trasmitter() {
    alarm_config.handler = handle_disc;
    (void)signal(SIGALRM, alarm_config.handler);
    alarm_config.count = 0;

    if (send_supervision_frame(transmitter.fd, TX_ADDRESS, DISC_CONTROL)) {
        return 1;
    }
    alarm(alarm_config.timeout);

    int flag = 0;
    for (;;) {
        if (read_supervision_frame(transmitter.fd, RX_ADDRESS, DISC_CONTROL) == 0) {
            flag = 1;
            break;
        }

        if (alarm_config.count == alarm_config.num_retransmissions) {
            break;
        }
    }
    alarm(0);

    if (!flag) {
        return 2;
    }

    if (send_supervision_frame(transmitter.fd, TX_ADDRESS, UA_CONTROL)) {
        return 3;
    }

    return 0;
}
