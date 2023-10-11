#include "common.h"

#include <stdlib.h>
#include <unistd.h>

// TODO: Refactor this to just build so that we can have a reusable alarm handler (using data holder)
// TODO: make send and read for information frames

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

int read_supervision_frame(int fd, uint8_t address, uint8_t control, uint8_t* rej_ctrl) {
    uint8_t byte;
    state_t state = START;

    uint8_t is_rej;

    while (state != STOP) {
        if (read(fd, &byte, 1) != 1) {
            continue;
        }
        if (state == START) {
            if (byte == FLAG) {
                state = FLAG_RCV;
            }
        } else if (state == FLAG_RCV) {
            is_rej = 0;
            if (byte == address) {
                state = A_RCV;
            } else if (byte != FLAG) {
                state = START;
            }
        } else if (state == A_RCV) {
            if (rej_ctrl != NULL) {
                if (byte == *rej_ctrl) {
                    is_rej = 1;
                }
            }
            if (byte == control || is_rej) {
                state = C_RCV;
            } else if (byte == FLAG) {
                state = FLAG_RCV;
            } else {
                state = START;
            }
        } else if (state == C_RCV) {
            if ((!is_rej && byte == (address ^ control)) || (is_rej && byte == (address ^ *rej_ctrl))) {
                state = BCC_OK;
            } else if (byte == FLAG) {
                state = FLAG_RCV;
            } else {
                state = START;
            }
        } else if (state == BCC_OK) {
            if (byte == FLAG) {
                if (is_rej) {
                    return 1;
                }
                state = STOP;
            } else {
                state = START;
            }
        }
    }

    return 0;
}

int read_information_frame(int fd, uint8_t address, uint8_t control, uint8_t repeated_ctrl, uint8_t* data, int* data_size) {
    uint8_t byte;
    state_t state = START;

    uint8_t is_repeated;
    *data_size = 0;

    while (state != STOP) {
        if (read(fd, &byte, 1) != 1) {
            continue;
        }
        if (state == START) {
            if (byte == FLAG) {
                state = FLAG_RCV;
            }
        } else if (state == FLAG_RCV) {
            is_repeated = 0;
            if (byte == address) {
                state = A_RCV;
            } else if (byte != FLAG) {
                state = START;
            }
        } else if (state == A_RCV) {
            if (byte == repeated_ctrl) {
                is_repeated = 1;
            }
            if (byte == control || is_repeated) {
                state = C_RCV;
            } else if (byte == FLAG) {
                state = FLAG_RCV;
            } else {
                state = START;
            }
        } else if (state == C_RCV) {
            if ((!is_repeated && byte == (address ^ control)) || (is_repeated && byte == (address ^ repeated_ctrl))) {
                state = BCC_OK;
            } else if (byte == FLAG) {
                state = FLAG_RCV;
            } else {
                state = START;
            }
        } else if (state == BCC_OK) {
            if (byte == FLAG) {
                state = STOP;
                if (is_repeated) {
                    return 1;
                }
            } else {
                data[(*data_size)++] = byte;
            }
        }
    }

    return 0;
}
