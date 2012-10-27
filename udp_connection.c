#define _GNU_SOURCE

#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "udp_connection.h"

#include "main.h"
#include "tools.h"
#include "queue.h"

void udp_signal_handler(int sig) {
    if (sig != SIGUSR1) {
        fprintf(stderr, "UDP:%d != %d\n", sig, SIGUSR1);
    } else {
        fprintf(stderr, "UDP:SIGUSR1\n");
    }
}

void* udp_receiver(void *init) {
    int str_length = 0;
    ssize_t result;

#ifdef DEBUG
    char straddr[INET6_ADDRSTRLEN];
#endif

    char buffer[BUFLEN];
    struct sockaddr_in6 udp_remote;
    socklen_t udp_remote_length;

    struct packet_data *received_data = NULL;

    int udp_sockfd = 0;
    struct queue *udp_receiver_queue = NULL;

    signal(SIGUSR1, udp_signal_handler);

    udp_sockfd = ((struct udp_connection*) init)->udp_sockfd;
    udp_receiver_queue = ((struct udp_connection*) init)->udp_receiver_queue;
    DELETE(init);

    while ((result = recvfrom(udp_sockfd, buffer, BUFLEN, 0, (struct sockaddr*) (&udp_remote), &udp_remote_length)) > 0) {
        received_data = NEW(struct packet_data);

        str_length = tools_str_length(buffer);

        received_data->remote = NEW(struct sockaddr_in6);
        received_data->frame = NEW_ARRAY(char, str_length);

        tools_memcpy(received_data->remote, &udp_remote, sizeof (udp_remote));
        tools_memcpy(received_data->frame, buffer, sizeof (char) * str_length);

#ifdef DEBUG
        inet_ntop(AF_INET6, &udp_remote.sin6_addr, straddr, sizeof (straddr));
        fprintf(stderr, "UDP:Receiver\nRemote: %s:%d\nData:\n---\n%s\n---\n", straddr, ntohs(udp_remote.sin6_port), buffer);
#endif

        queue_add(received_data, udp_receiver_queue);

        CLEAR(buffer, sizeof (char) *BUFLEN);
    }
    perror("recvfrom");

    return NULL;
}

void* udp_sender(void *init) {
    int str_length = 0;

#ifdef DEBUG
    char straddr[INET6_ADDRSTRLEN];
#endif

    struct packet_data *send_data = NULL;

    int udp_sockfd = 0;
    struct queue *udp_sender_queue = NULL;

    signal(SIGUSR1, udp_signal_handler);

    udp_sockfd = ((struct udp_connection*) init)->udp_sockfd;
    udp_sender_queue = ((struct udp_connection*) init)->udp_sender_queue;
    DELETE(init);

    while ((send_data = (struct packet_data*) queue_get(udp_sender_queue)) != NULL) {
        str_length = tools_str_length(send_data->frame);
        if (sendto(udp_sockfd, send_data->frame, str_length, 0, (struct sockaddr*) send_data->remote, sizeof (*(send_data->remote))) < 0) {
            perror("sendto");
        }
#ifdef DEBUG
        else {
            inet_ntop(AF_INET6, &(send_data->remote->sin6_addr), straddr, sizeof (straddr));
            printf("UDP:Sender\nRemote %s:%d\nData:\n---\n%s\n---\n", straddr, ntohs(send_data->remote->sin6_port), send_data->payload);
        }
#endif
    }

    return NULL;
}

struct udp_connection* udp_start(int port) {

    struct udp_connection *connection = NULL;

    if (port <= 0) {
        fprintf(stderr, "Port %d is not a valid value.\n", port);
        return NULL;
    }

    connection = NEW(struct udp_connection);
    if ((connection->udp_sockfd = socket(PF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("UDP: socket");
        exit(1);
    }

    connection->udp_local->sin6_family = AF_INET6;
    connection->udp_local->sin6_port = htons(port);
    connection->udp_local->sin6_addr = in6addr_any;

    if (bind(connection->udp_sockfd, (const struct sockaddr*) (&connection->udp_local), sizeof (connection->udp_local)) < 0) {
        perror("UDP: bind");
        exit(1);
    }

    connection->udp_sender_queue = queue_new();
    connection->udp_receiver_queue = queue_new();

    pthread_create(&connection->udp_receiver_thread, NULL, udp_receiver, connection);
    pthread_create(&connection->udp_sender_thread, NULL, udp_sender, connection);

    return connection;
}

void udp_stop(struct udp_connection *connection) {
    if (connection == NULL) {
        fprintf(stderr, "UDP connection is null.\n");
        return;
    }

    pthread_kill(connection->udp_receiver_thread, SIGUSR1);
    pthread_kill(connection->udp_sender_thread, SIGUSR1);
    pthread_join(connection->udp_receiver_thread, NULL);
    pthread_join(connection->udp_sender_thread, NULL);

    queue_delete(connection->udp_receiver_queue);
    queue_delete(connection->udp_sender_queue);
    connection->udp_receiver_queue = NULL;
    connection->udp_sender_queue = NULL;

    /* FIXME closing socket */
    shutdown(connection->udp_sockfd, SHUT_RDWR);
    close(connection->udp_sockfd);

    DELETE(connection);
}