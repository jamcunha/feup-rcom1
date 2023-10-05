#ifndef _FRAME_UTILS_H_
#define _FRAME_UTILS_H_

#define FLAG            0x7E
#define TX_ADDRESS      0x03
#define RX_ADDRESS      0x01
#define SET_CONTROL    0x03
#define UA_CONTROL     0x07
#define RR0_CONTROL    0x05
#define RR1_CONTROL    0x85
#define REJ0_CONTROL   0x01
#define REJ1_CONTROL   0x81
#define DISC_CONTROL   0x0B
#define I0_CONTROL     0x00
#define I1_CONTROL     0x40

#include <stdint.h>

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP,
} state_t;

// TODO: think more about build_supervision_frame

// Builds a supervision frame with the given address and control
// Returns a pointer to the frame
// We don't send the frame here to be easier to write to the serial port (alarm handling and stuff)
uint8_t* build_supervision_frame(uint8_t address, uint8_t control);

int read_supervision_frame(int fd, uint8_t address, uint8_t control);

#endif // _FRAME_UTILS_H_
