#define _GNU_SOURCE

#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#ifdef DEBUG
#include <arpa/inet.h>
#endif

#include "tcp_connection.h"

#include "main.h"
#include "tools.h"
#include "queue.h"

void tcp_signal_handler(int sig) {
    if (sig != SIGUSR1) {
        fprintf(stderr, "UDP:%d != %d\n", sig, SIGUSR1);
    } else {
        fprintf(stderr, "UDP:SIGUSR1\n");
    }
}

void* tcp_receiver(void *init) {
    int str_length = 0;
    ssize_t result;

#ifdef DEBUG
    char straddr[INET_ADDRSTRLEN];
#endif

    char buffer[BUFLEN];

    struct packet_data *send_data = NULL;

    int tcp_sockfd = 0;
    struct queue *tcp_receiver_queue = NULL;
    struct sockaddr_in6 *udp_remote = NULL;
#ifdef DEBUG
    struct sockaddr_in *tcp_remote = NULL;
#endif

    signal(SIGUSR1, tcp_signal_handler);

    tcp_sockfd = ((struct tcp_connection*) init)->tcp_sockfd;
    tcp_receiver_queue = ((struct tcp_connection*) init)->tcp_receiver_queue;
    udp_remote = ((struct tcp_connection*) init)->udp_remote;
#ifdef DEBUG
    tcp_remote = ((struct tcp_connection*) init)->tcp_remote;
#endif

    while ((result = recv(tcp_sockfd, buffer, BUFLEN, 0)) > 0) {
        send_data = NEW(struct packet_data);

        str_length = tools_str_length(buffer);

        send_data->remote = NEW(struct sockaddr_in6);
        send_data->frame = NEW_ARRAY(char, str_length);

        tools_memcpy(send_data->remote, udp_remote, sizeof (*udp_remote));
        tools_memcpy(send_data->frame, buffer, sizeof (char) * str_length);

#ifdef DEBUG
        inet_ntop(AF_INET, &tcp_remote->sin_addr, straddr, sizeof (straddr));
        fprintf(stderr, "TCP:Receiver\nRemote: %s:%d\nData:\n---\n%s\n---\n", straddr, ntohs(tcp_remote->sin_port), buffer);
#endif

        queue_add(send_data, tcp_receiver_queue);

        CLEAR(buffer, sizeof (char) *BUFLEN);
    }
    perror("TCP: recv");

    DELETE(init);
    return NULL;
}

void* tcp_sender(void *init) {
    int str_length = 0;

#ifdef DEBUG
    char straddr[INET_ADDRSTRLEN];
#endif

    struct packet_data *send_data = NULL;

    int tcp_sockfd = 0;
    struct queue *tcp_sender_queue = NULL;
#ifdef DEBUG
    struct sockaddr_in *tcp_remote = NULL;
#endif

    signal(SIGUSR1, tcp_signal_handler);

    tcp_sockfd = ((struct tcp_connection*) init)->tcp_sockfd;
    tcp_sender_queue = ((struct tcp_connection*) init)->tcp_sender_queue;
#ifdef DEBUG
    tcp_remote = ((struct tcp_connection*) init)->tcp_remote;
#endif

    while ((send_data = (struct packet_data*) queue_get(tcp_sender_queue)) != NULL) {
        str_length = tools_str_length(send_data->frame);
        if (send(tcp_sockfd, send_data->frame, str_length, 0) < 0) {
            perror("TCP: send");
        }
#ifdef DEBUG
        else {
            inet_ntop(AF_INET, &(tcp_remote->sin_addr), straddr, sizeof (straddr));
            printf("TCP:Sender\nRemote %s:%d\nData:\n---\n%s\n---\n", straddr, ntohs(tcp_remote->sin_port), send_data->frame);
        }
#endif
    }

    DELETE(init);
    return NULL;
}

struct tcp_connection* tcp_start(char *hostname, int port, struct sockaddr_in6 *udp_remote, struct queue *udp_sender_queue) {

    struct tcp_connection *connection = NULL;
    struct hostent *host;

    if (hostname == NULL || port <= 0) {
        fprintf(stderr, "Hostname %s is null or port %d is not a valid value.\n", hostname, port);
        return NULL;
    }

    connection = NEW(struct tcp_connection);
    if ((connection->tcp_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("TCP: socket");
        exit(1);
    }


    connection->tcp_remote->sin_family = AF_INET;
    connection->tcp_remote->sin_port = htons(port);

    host = gethostbyname(hostname);
    tools_memcpy(&connection->tcp_remote->sin_addr.s_addr,
            host->h_addr, host->h_length);

    if (connect(connection->tcp_sockfd, (const struct sockaddr*) &connection->tcp_remote, sizeof (connection->tcp_remote)) < 0) {
        perror("TCP: connect");
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
        fprintf(stderr, "TCP connection is null.\n");
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
    shutdown(connection->tcp_sockfd, SHUT_RDWR);
    close(connection->tcp_sockfd);

    DELETE(connection);
}
