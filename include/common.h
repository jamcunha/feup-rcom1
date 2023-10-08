#ifndef _COMMON_H_
#define _COMMON_H_

// maybe have a macro for RR, REJ and I

#define FLAG            0x7E
#define TX_ADDRESS      0x03
#define RX_ADDRESS      0x01
#define SET_CONTROL     0x03
#define UA_CONTROL      0x07
#define RR0_CONTROL     0x05 // bit: 00000101
#define RR1_CONTROL     0x85 // bit: 10000101
#define REJ0_CONTROL    0x01 // bit: 00000001
#define REJ1_CONTROL    0x81 // bit: 10000001
#define DISC_CONTROL    0x0B
#define I0_CONTROL      0x00 // bit: 00000000
#define I1_CONTROL      0x40 // bit: 01000000
#define ESC             0x7D

#include <stdint.h>

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP,
} state_t;

typedef struct {
    int count;
    int timeout;
    int num_retransmissions;
} alarm_t;

int send_supervision_frame(int fd, uint8_t address, uint8_t control);

int read_supervision_frame(int fd, uint8_t address, uint8_t control);

#endif // _COMMON_H_
