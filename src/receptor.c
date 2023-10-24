#include "receptor.h"

#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "frame_utils.h"

struct {
    struct termios oldtio, newtio;
} receptor;

int receptor_num = 1;

void alarm_handler_receptor(int signo) {
    alarm_config.count++;
    if (write(data_holder.fd, data_holder.buffer, data_holder.length) != data_holder.length) {
        return;
    }
    alarm(alarm_config.timeout);

    // if alarm count is > than num_retransmissions,
    // it will try to write one more time but it will fail
    if (alarm_config.count <= alarm_config.num_retransmissions) {
        printf("Alarm #%d\n", alarm_config.count);
    }
}

int open_receptor(char* serial_port, int baudrate, int timeout, int nRetransmissions) {
    alarm_config.count = 0;
    alarm_config.timeout = timeout;
    alarm_config.num_retransmissions = nRetransmissions;

    (void)signal(SIGALRM, alarm_handler_receptor);

    data_holder.fd = open(serial_port, O_RDWR | O_NOCTTY);
    if (data_holder.fd < 0) {
        return 1;
    }

    if (tcgetattr(data_holder.fd, &receptor.newtio) == -1) {
        return 2;
    }

    memset(&receptor.newtio, 0, sizeof(receptor.newtio));

    receptor.newtio.c_cflag = baudrate | CS8 | CLOCAL | CREAD;
    receptor.newtio.c_iflag = IGNPAR;
    receptor.newtio.c_oflag = 0;

    receptor.newtio.c_lflag = 0;
    receptor.newtio.c_cc[VTIME] = 0;
    receptor.newtio.c_cc[VMIN] = 1;

    tcflush(data_holder.fd, TCIOFLUSH);

    if (tcsetattr(data_holder.fd, TCSANOW, &receptor.newtio) == -1) {
        return 3;
    }

    return 0;
}

int close_receptor() {
    if (tcdrain(data_holder.fd) == -1) {
        return 1;
    }

    if (tcsetattr(data_holder.fd, TCSANOW, &receptor.newtio) == -1) {
        return 2;
    }

    close(data_holder.fd);
    return 0;
}

int connect_receptor() {
    while (read_supervision_frame(TX_ADDRESS, SET_CONTROL, NULL) != 0) {}

    if (send_receiver_frame(UA_CONTROL)) {
        return 1;
    }

    return 0;
}

int disconnect_receptor() {
    while (read_supervision_frame(TX_ADDRESS, DISC_CONTROL, NULL) != 0) {}

    if (send_transmitter_frame(DISC_CONTROL, NULL, 0)) {
        return 1;
    }

    int res = -1;
    while (res != 0) {
        res = read_supervision_frame(RX_ADDRESS, UA_CONTROL, NULL);
        if (res == 1) {
            break;
        }
    }
    alarm(0);

    if (res == 1) {
        return 2;
    }

    return 0;
}

int receive_packet(uint8_t* packet) {
    // TODO: PROBLEM HERE -> RETURNING 0 WITH CORRUPTED BYTES (and bcc2 is not catching it)
    if (read_information_frame(TX_ADDRESS, I_CONTROL(1 - receptor_num), I_CONTROL(receptor_num)) != 0) {
        if (send_receiver_frame(RR_CONTROL(1 - receptor_num))) {
            return -1;
        }

        return 0;
    }

    uint8_t data[DATA_SIZE];
    uint8_t bcc2;
    size_t data_size = destuff_data(data_holder.buffer, data_holder.length, data, &bcc2);

    uint8_t tmp_bcc2 = 0;
    for (size_t i = 0; i < data_size; i++) {
        tmp_bcc2 ^= data[i];
    }

    if (tmp_bcc2 != bcc2) {
        if (send_receiver_frame(REJ_CONTROL(1 - receptor_num))) {
            return -1;
        }

        return 0;
    }

    memcpy(packet, data, data_size);
    if (send_receiver_frame(RR_CONTROL(receptor_num))) {
        return -1;
    }

    receptor_num = 1 - receptor_num;
    return data_size;
}
