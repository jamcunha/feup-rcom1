// Link layer protocol implementation

#include "link_layer.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "frame_utils.h"
#include "transmitter.h"
#include "receptor.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

LinkLayerRole role;

struct timeval start_time, end_time;

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
        if (open_receptor(connectionParameters.serialPort,
                    connectionParameters.baudRate,
                    connectionParameters.timeout,
                    connectionParameters.nRetransmissions)) {
            return -1;
        }
        role = LlRx;

        if (connect_receptor()) {
            return -1;
        }
    }

    stats.total_packets = 0;
    stats.accepted_packets = 0;
    stats.rejected_packets = 0;

    gettimeofday(&start_time, NULL);
    return 1;
}

int llwrite(const unsigned char *buf, int bufSize) {
    if (role == LlRx) {
        return 1;
    }

    if (send_packet(buf, bufSize)) {
        return -1;
    }

    return 0;
}

int llread(unsigned char *packet) {
    if (role == LlTx) {
        return 1;
    }

    unsigned char data[DATA_SIZE];
    int size;
    if ((size = receive_packet(data)) < 0) {
        return -1;
    }
    memcpy(packet, data, size);

    return size;
}

int llclose(int showStatistics) {
    gettimeofday(&end_time, NULL);

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

    if (showStatistics) {
        long elapsed_sec = end_time.tv_sec - start_time.tv_sec;
        long elapsed_usec = end_time.tv_usec - start_time.tv_usec;
        double elapsed_time = elapsed_sec + elapsed_usec / 10000000.0;

        printf("\n");
        printf("------------------------ STATISTICS ------------------------\n");
        printf("Total packets: %d\n", stats.total_packets);
        printf("\n");
        printf("Accepted packets: %d\n", stats.accepted_packets);
        printf("Rejected packets: %d\n", stats.rejected_packets);
        printf("\n");
        printf("Accepted percentage: %.2f%%\n", (float) stats.accepted_packets / stats.total_packets * 100);
        printf("Rejected percentage: %.2f%%\n", (float) stats.rejected_packets / stats.total_packets * 100);
        printf("------------------------------------------------------------\n");
        printf("Total time: %lf seconds\n", elapsed_time);
        printf("------------------------------------------------------------\n");
        printf("\n");
    }

    return 1;
}
