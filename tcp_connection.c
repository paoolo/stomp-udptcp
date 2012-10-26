#define _XOPEN_SOURCE
#define _POSIX_SOURCE

#include "tcp_connection.h"

#include <signal.h>

void tcp_signal_handler(int sig) {
    if (sig != SIGUSR1) {
        fprintf(stderr, "This is not a signal: %d != %d\n", sig, SIGUSR1);
    } else {
        fprintf(stderr, "Received signal SIGUSR1\n");
    }
}

struct tcp_connection* tcp_start(char *hostname, int port, struct sockaddr_in6 *udp_remote, struct queue *udp_sender_queue) {
    struct tcp_connection *connection = NULL;
    struct sockaddr_in remote;
    struct hostent *host;

    if (hostname == NULL || port <= 0) {
        return NULL;
    }

    connection = NEW(struct tcp_connection);
    if ((connection->sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    host = gethostbyname(hostname);
    bzero((char*)&remote, sizeof(remote));
    remote.sin_family = AF_INET;
    bcopy((char*)host->h_addr, (char*)&remote.sin_addr.s_addr, host->h_length);
    remote.sin_port = htons(port);

    if (connect(connection->sockfd,(struct sockaddr*)&remote, sizeof(remote)) < 0) {
        perror("connect");
        exit(1);
    }

    connection->tcp_sender_queue = queue_new();
    connection->tcp_receiver_queue = udp_sender_queue;
    connection->udp_remote = udp_remote;

    pthread_create(&connection->tcp_receiver_thread, NULL, tcp_receiver, connection);
    pthread_create(&connection->tcp_sender_thread, NULL, tcp_sender, connection);

    return connection;
}

void tcp_stop(struct tcp_connection *connection) {
    if (connection == NULL) {
        return;
    }

    pthread_kill(connection->tcp_receiver_thread, SIGUSR1);
    pthread_kill(connection->tcp_sender_thread, SIGUSR1);
    pthread_join(connection->tcp_receiver_thread, NULL);
    pthread_join(connection->tcp_sender_thread, NULL);

    queue_delete(connection->tcp_receiver_queue);
    queue_delete(connection->tcp_sender_queue);
    connection->tcp_receiver_queue = NULL;
    connection->tcp_sender_queue = NULL;

    /* FIXME closing socket */
    shutdown(connection->sockfd, SHUT_RDWR);
    close(connection->sockfd);

    DELETE(connection);
}

void* tcp_receiver(void *init) {
    int str_length = 0;
    ssize_t result;

    char buffer[BUFLEN];

    struct udp_data *send_data = NULL;

    int tcp_sockfd = 0;
    struct queue *tcp_receiver_queue = NULL;
    struct sockaddr_in6 *udp_remote = NULL;

    signal(SIGUSR1, tcp_signal_handler);

    tcp_sockfd = ((struct tcp_connection*)init)->tcp_sockfd;
    tcp_receiver_queue = ((struct tcp_connection*)init)->tcp_receiver_queue;
    udp_remote = ((struct tcp_connection*)init)->udp_remote;
    DELETE(init);

    while ((result = recv(tcp_sockfd, buffer, BUFLEN, 0)) > 0) {
        send_data = NEW(struct udp_data);

        str_length = tools_str_length(buffer);

        send_data->remote = NEW(struct sockaddr_in6);
        send_data->payload = NEW_ARRAY(char, str_length);

        tools_memcpy(send_data->remote, &udp_remote, sizeof(udp_remote));
        tools_memcpy(send_data->payload, buffer, sizeof(char) * str_length);

        queue_add(send_data, tcp_receiver_queue);

        CLEAR(buffer, sizeof(char)*BUFLEN);
    }
    perror("recv");

    return NULL;
}

void* tcp_sender(void *init) {
    int str_length = 0;
    
    struct udp_data *send_data = NULL;

    int tcp_sockfd = 0;
    struct queue *tcp_sender_queue = NULL;

    signal(SIGUSR1, tcp_signal_handler);

    tcp_sockfd = ((struct tcp_connection*)init)->tcp_sockfd;
    tcp_sedner_queue = ((struct tcp_connection*)init)->tcp_sender_queue;
    DELETE(init);

    while ((send_data = (struct udp_data*)queue_get(tcp_sender_queue)) != NULL) {
        str_length = tools_str_length(send_data->payload);
        if (send(sockfd, send_data->payload, str_length, 0) < 0) {
            perror("send");
        }
#ifdef DEBUG
        else {
            inet_ntop(AF_INET6, &(send_data->remote->sin6_addr), straddr, sizeof(straddr));
            printf("Packet from %s:%d sent to broker.\nData:\n---\n%s\n---\n", straddr, ntohs(send_data->remote->sin6_port), send_data->payload);
        }
#endif
    }

    return NULL;
}
