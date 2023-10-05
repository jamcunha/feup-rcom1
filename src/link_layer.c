// Link layer protocol implementation

#include "link_layer.h"

#include <signal.h>
#include <unistd.h>

#include "transmitter.h"
#include "receptor.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

// TODO: REMOVE
#include <stdio.h>

LinkLayerRole role;

int alarm_count = 0;
int alarm_timeout = 0;
void alarm_handler(int signo) {
    alarm_count++;

    sendSET();
    alarm(alarm_timeout);

    printf("Alarm #%d\n", alarm_count); // TODO: REMOVE
}

int llopen(LinkLayer connectionParameters) {
    if (connectionParameters.role == LlTx) {
        if (open_transmitter(connectionParameters.serialPort, connectionParameters.baudRate)) {
            return -1;
        }
        role = LlTx;

        // Setup alarm
        alarm_timeout = connectionParameters.timeout;
        (void)signal(SIGALRM, alarm_handler);

        sendSET();
        alarm(alarm_timeout);

        int flag = FALSE;
        while (TRUE) {
            if (receivedUA() == 0) {
                flag = TRUE;
                break;
            }

            // Ask teacher if is 1st try + 3 alarm tries or 3 tries as a whole
            if (alarm_count == connectionParameters.nRetransmissions) {
                break;
            }
        }
        alarm(0);

        if (!flag) {
            printf("UA not received\n"); // TODO: REMOVE
            return -1;
        }
    } else if (connectionParameters.role == LlRx) {
        if (open_receptor(connectionParameters.serialPort, connectionParameters.baudRate)) {
            return -1;
        }
        role = LlRx;

        while (receivedSET() != 0) {}

        if (sendUA()) {
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
        if (close_transmitter()) {
            return -1;
        }
    } else if (role == LlRx) {
        if (close_receptor()) {
            return -1;
        }
    }

    return 1;
}
