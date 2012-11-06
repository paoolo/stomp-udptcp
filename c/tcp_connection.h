#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

struct tcp_connection {
    int
        tcp_sockfd;

    struct sockaddr_in6
        *udp_remote;
    
    struct sockaddr_in
        *tcp_remote;

    struct queue
        *tcp_receiver_queue, *tcp_sender_queue;

    pthread_t
        tcp_receiver_thread, tcp_sender_thread;
};

struct tcp_connection* tcp_start(char *hostname, int port, struct sockaddr_in6 *udp_remote, struct queue *udp_sender_queue);

void tcp_stop(struct tcp_connection *connection);

#endif
