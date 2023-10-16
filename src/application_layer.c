// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

#include <stdio.h>
#include <string.h>

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    if (strncmp(serialPort, "/dev/ttyS", 9) != 0) {
        fprintf(stderr, "Invalid serial port: %s\n", serialPort);
        return;
    }

    if ((strcmp(role, "tx") != 0) && (strcmp(role, "rx") != 0)) {
        fprintf(stderr, "Invalid role: %s\n", role);
        return;
    }

    LinkLayer link_layer;
    strncpy(link_layer.serialPort, serialPort, 50);
    link_layer.role = strcmp(role, "tx") == 0 ? LlTx : LlRx;
    link_layer.baudRate = baudRate;
    link_layer.nRetransmissions = nTries;
    link_layer.timeout = timeout;

    if (llopen(link_layer) == -1) {
        fprintf(stderr, "Failed to establish connection\n");
        return;
    }
    printf("Connection established\n");

    // not supposed to send this but its for testing purposes
    if (llwrite((unsigned char *) filename, strlen(filename)) == -1) {
        fprintf(stderr, "Failed to send filename\n");
        return;
    }

    int size;
    unsigned char buf[256];
    while ((size = llread(buf)) == -1) { }

    if (llwrite((unsigned char *) filename, strlen(filename)) == -1) {
        fprintf(stderr, "Failed to send filename\n");
        return;
    }

    while ((size = llread(buf)) == -1) { }

    if (llclose(0) == -1) {
        fprintf(stderr, "Failed to close connection\n");
        return;
    }
    printf("Connection closed\n");
}
