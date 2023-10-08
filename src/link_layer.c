// Link layer protocol implementation

#include "link_layer.h"

#include <signal.h>
#include <unistd.h>

#include "transmitter.h"
#include "receptor.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

LinkLayerRole role;

int llopen(LinkLayer connectionParameters) {
    if (connectionParameters.role == LlTx) {
        if (open_transmitter(connectionParameters.serialPort,
                    connectionParameters.baudRate,
                    connectionParameters.timeout,
                    connectionParameters.nRetransmissions)) {
            return -1;
        }
        role = LlTx;

        if (connect_trasmitter()) {
            return -1;
        }
    } else if (connectionParameters.role == LlRx) {
        if (open_receptor(connectionParameters.serialPort, connectionParameters.baudRate)) {
            return -1;
        }
        role = LlRx;

        if (connect_receptor()) {
            return -1;
        }
    }

    return 1;
}

int llwrite(const unsigned char *buf, int bufSize) {
    // TODO

    return 0;
}

int llread(unsigned char *packet) {
    // TODO

    return 0;
}

int llclose(int showStatistics) {
    // TODO find what is the statistics

    if (role == LlTx) {
        if (disconnect_trasmitter()) {
            return -1;
        }

        if (close_transmitter()) {
            return -1;
        }
    } else if (role == LlRx) {
        if (disconnect_receptor()) {
            return -1;
        }

        if (close_receptor()) {
            return -1;
        }
    }

    return 1;
}
