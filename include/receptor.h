#ifndef _EMISSOR_H_
#define _EMISSOR_H_

int open_receptor(char* serial_port, int baudrate);

int close_receptor();

int connect_receptor();

#endif // _EMISSOR_H_
