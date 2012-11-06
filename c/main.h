#ifndef MAIN_H
#define MAIN_H

#define BUFLEN 1024

struct packet_data {
    struct sockaddr_in6 *remote;
    char *frame;
};

#endif
