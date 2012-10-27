#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include <getopt.h>

#include "main.h"

#include "tools.h"
#include "queue.h"
#include "map.h"
#include "bridge.h"

#include "tcp_connection.h"
#include "udp_connection.h"

int map_compare(char *object, char *another) {
    return strcmp(object, another);
}

int string_to_int(char *str) {
    int result = 0;

    while (str != NULL && *str != 0) {
        result = 10 * result + (*str - '0');
        str++;
    }

    return result;
}

void usage(char **argv) {
    fprintf(stderr, "Usage: %s -t|--tcphost <string> -T|--tcpport <int> | -U|--udpport <int>\n", argv[0]);
}

int main(int argc, char **argv) {

    struct udp_connection *udp_connection;

    char *tcp_hostname = NULL;
    int udp_port = -1, tcp_port = -1;

    const char* const short_options = "t:T:U:";
    const struct option long_options[] = {
        {"tcphost", 1, NULL, 't'},
        {"tcpport", 1, NULL, 'T'},
        {"udpport", 1, NULL, 'U'},
        {NULL, 0, NULL, 0}
    };

    int next_option;
    do {
        next_option = getopt_long(argc, argv, short_options, long_options, NULL);
        switch (next_option) {
            case 't':
                tcp_hostname = NEW_ARRAY(char, strlen(optarg));
                memcpy(tcp_hostname, optarg, strlen(optarg));
                break;
            case 'T':
                tcp_port = string_to_int(optarg);
                break;
            case 'U':
                udp_port = string_to_int(optarg);
                break;
            case '?':
                usage(argv);
                exit(-1);
                break;
            case -1:
                break;
            default:
                break;
        }
    } while (next_option != -1);

    if (tcp_hostname == NULL || tcp_port <= 0 || udp_port <= 0) {
        usage(argv);
        exit(1);
    }

    udp_connection = udp_start(udp_port);

    pthread_join(udp_connection->udp_receiver_thread, NULL);
    pthread_join(udp_connection->udp_sender_thread, NULL);

    return 0;
}
