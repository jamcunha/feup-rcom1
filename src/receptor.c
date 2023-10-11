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

int receptor_num = 1;

int destuff_data(const uint8_t* stuffed_data, size_t length, uint8_t* data, uint8_t* bcc2) {
    uint8_t destuffed_data[DATA_SIZE + 1];
    size_t idx = 0;

    for (size_t i = 0; i < length; i++) {
        if (stuffed_data[i] == ESC) {
            i++;
            destuffed_data[idx++] = stuffed_data[i] ^ 0x20;
        } else {
            destuffed_data[idx++] = stuffed_data[i];
        }
    }

    *bcc2 = destuffed_data[idx - 1];

    memcpy(data, destuffed_data, idx - 1);
    return idx - 1;
}

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
    while (read_supervision_frame(receptor.fd, TX_ADDRESS, SET_CONTROL, NULL) != 0) {}

    if (send_supervision_frame(receptor.fd, RX_ADDRESS, UA_CONTROL)) {
        return 1;
    }

    return 0;
}

int disconnect_receptor() {
    while (read_supervision_frame(receptor.fd, TX_ADDRESS, DISC_CONTROL, NULL) != 0) {}

    if (send_supervision_frame(receptor.fd, RX_ADDRESS, DISC_CONTROL)) {
        return 1;
    }

    while (read_supervision_frame(receptor.fd, TX_ADDRESS, UA_CONTROL, NULL) != 0) {}

    return 0;
}

int receive_packet(unsigned char* packet) {
    uint8_t stuffed_data[STUFFED_DATA_SIZE];
    int stuffed_size = 0;

    if (read_information_frame(receptor.fd, TX_ADDRESS, I_CONTROL(1 - receptor_num), I_CONTROL(receptor_num), stuffed_data, &stuffed_size) != 0) {
        send_supervision_frame(receptor.fd, RX_ADDRESS, RR_CONTROL(1 - receptor_num));
        return 0;
    }

    uint8_t data[DATA_SIZE];
    uint8_t bcc2;
    size_t data_size = destuff_data(stuffed_data, stuffed_size, data, &bcc2);

    uint8_t tmp_bcc2 = 0;
    for (size_t i = 0; i < data_size; i++) {
        tmp_bcc2 ^= data[i];
    }

    if (tmp_bcc2 != bcc2) {
        send_supervision_frame(receptor.fd, RX_ADDRESS, REJ_CONTROL(1 - receptor_num));
        return 0;
    }

    memcpy(packet, data, data_size);

    send_supervision_frame(receptor.fd, RX_ADDRESS, RR_CONTROL(receptor_num));
    receptor_num = 1 - receptor_num;
    return data_size;
}
