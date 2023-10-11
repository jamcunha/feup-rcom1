#ifndef _COMMON_H_
#define _COMMON_H_

#define FLAG            0x7E
#define TX_ADDRESS      0x03
#define RX_ADDRESS      0x01
#define SET_CONTROL     0x03
#define UA_CONTROL      0x07
#define RR_CONTROL(n)   ((1 << (7*n)) | 0x05 )
#define REJ_CONTROL(n)  ((1 << (7*n)) | 0x01 )
#define DISC_CONTROL    0x0B
#define I_CONTROL(n)    ((1 << (6*n)) & 0x40)
#define ESC             0x7D

#define DATA_SIZE           1024
#define STUFFED_DATA_SIZE   (DATA_SIZE * 2 + 2)

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

// TODO: think about data holder here

int send_supervision_frame(int fd, uint8_t address, uint8_t control);

int read_supervision_frame(int fd, uint8_t address, uint8_t control, uint8_t* rej_byte);

// maybe return data size
int read_information_frame(int fd, uint8_t address, uint8_t control, uint8_t repeated_ctrl, uint8_t* data, int* data_size);

#endif // _COMMON_H_
