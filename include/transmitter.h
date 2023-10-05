#ifndef _TRANSMITTER_H_
#define _TRANSMITTER_H_

int open_transmitter(char* serial_port, int baudrate);

int close_transmitter();

int sendSET();

int receivedUA();

#endif // _TRANSMITTER_H_
