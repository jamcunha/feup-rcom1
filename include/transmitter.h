#ifndef _TRANSMITTER_H_
#define _TRANSMITTER_H_

int open_transmitter(char* serial_port, int baudrate, int timeout, int nRetransmissions);

int close_transmitter();

int connect_trasmitter();

#endif // _TRANSMITTER_H_
