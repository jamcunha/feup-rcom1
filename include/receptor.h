#ifndef _EMISSOR_H_
#define _EMISSOR_H_

int open_receptor(char* serial_port, int baudrate);

int close_receptor();

int receivedSET();

int sendUA();

#endif // _EMISSOR_H_
