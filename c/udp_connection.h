#ifndef UDP_CONNECTION_H
#define UDP_CONNECTION_H

struct udp_connection {
    int
        udp_sockfd;

    struct sockaddr_in6
        *udp_local;

    struct queue
        *udp_receiver_queue, *udp_sender_queue;

    pthread_t
        udp_receiver_thread, udp_sender_thread;
};

struct udp_connection* udp_start(int port);

void udp_stop(struct udp_connection *connection);

#endif
