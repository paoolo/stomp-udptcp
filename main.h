
#ifndef MAIN_H
#define MAIN_H

struct thread_init {
    int sockfd;
    struct queue *queue;
    struct sockaddr_in6 *remote;
};

struct udp_data {
    char *payload;
    struct sockaddr_in6 *remote;
};

struct tcp_data {
    char *payload;
};

#endif /* MAIN_H */
