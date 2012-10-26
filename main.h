
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

struct main_app {
    struct udp_connection
        *udp_connection;

    struct list
        *tcp_connections;
};

struct udp_connection {
    int
        udp_sockfd;

    struct queue
        *udp_receiver_queue, *udp_sender_queue;

    pthread_t
        udp_receiver_thread, udp_sender_thread;

};

#endif /* MAIN_H */
