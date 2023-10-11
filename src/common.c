#include "common.h"

#include <stdlib.h>
#include <unistd.h>

int send_supervision_frame(int fd, uint8_t address, uint8_t control) {
    uint8_t* frame = malloc(5 * sizeof(uint8_t));

    frame[0] = FLAG;
    frame[1] = address;
    frame[2] = control;
    frame[3] = address ^ control;
    frame[4] = FLAG;

    if (write(fd, frame, 5) != 5) {
        return 1;
    }

    // Wait untill all bytes have been written to the serial port
    sleep(1);

    free(frame);
    return 0;
}

int read_supervision_frame(int fd, uint8_t address, uint8_t control) {
    uint8_t byte;
    state_t state = START;

    while (state != STOP) {
        if (read(fd, &byte, 1) != 1) {
            continue;
        }
        if (state == START) {
            if (byte == FLAG) {
                state = FLAG_RCV;
            }
        } else if (state == FLAG_RCV) {
            if (byte == address) {
                state = A_RCV;
            } else if (byte != FLAG) {
                state = START;
            }
        } else if (state == A_RCV) {
            if (byte == control) {
                state = C_RCV;
            } else if (byte == FLAG) {
                state = FLAG_RCV;
            } else {
                state = START;
            }
        } else if (state == C_RCV) {
            if (byte == (address ^ control)) {
                state = BCC_OK;
            } else if (byte == FLAG) {
                state = FLAG_RCV;
            } else {
                state = START;
            }
        } else if (state == BCC_OK) {
            if (byte == FLAG) {
                state = STOP;
            } else {
                state = START;
            }
        }
    }

    return 0;
}
