#include "frame_utils.h"

#include <stdlib.h>
#include <unistd.h>

uint8_t* build_supervision_frame(uint8_t address, uint8_t control) {
    uint8_t* frame = malloc(5 * sizeof(uint8_t));

    frame[0] = FLAG;
    frame[1] = address;
    frame[2] = control;
    frame[3] = address ^ control;
    frame[4] = FLAG;

    return frame;
}

int read_supervision_frame(int fd, uint8_t address, uint8_t control) {
    uint8_t byte;
    state_t state = START;

    while (state != STOP) {
        if (read(fd, &byte, 1) != 1) {
            return 1;
        }

        switch (state) {
            case START:
                if (byte == FLAG) {
                    state = FLAG_RCV;
                }
                break;
            case FLAG_RCV:
                if (byte == address) {
                    state = A_RCV;
                } else if (byte != FLAG) {
                    state = START;
                }
                break;
            case A_RCV:
                if (byte == control) {
                    state = C_RCV;
                } else if (byte == FLAG) {
                    state = FLAG_RCV;
                } else {
                    state = START;
                }
                break;
            case C_RCV:
                if (byte == (address ^ control)) {
                    state = BCC_OK;
                } else if (byte == FLAG) {
                    state = FLAG_RCV;
                } else {
                    state = START;
                }
                break;
            case BCC_OK:
                if (byte == FLAG) {
                    state = STOP;
                } else {
                    state = START;
                }
                break;
            default:
                break;
        }
    }

    return 0;
}