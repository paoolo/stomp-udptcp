#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "tools.h"
#include "queue.h"
#include "main.h"

#define DEBUG
#define BUFLEN 512

/* UDP receiver thread.
 * It receives data from sensor and put it to queue to forward to broker via TCP socket. */
void* udp_receiver(void *data) {
    int str_length = 0;
    ssize_t result;

#ifdef DEBUG
    char straddr[INET6_ADDRSTRLEN];
#endif

    char buffer[BUFLEN];
    struct sockaddr_in6 remote;
    socklen_t length;

    struct udp_data *received_data = NULL;

    int sockfd = 0;
    struct queue *queue = NULL;

    sockfd = ((struct thread_init*)data)->sockfd;
    queue = ((struct thread_init*)data)->queue;
    DELETE(data);

    while ((result = recvfrom(sockfd, buffer, BUFLEN, 0, (struct sockaddr*)(&remote), &length)) > 0) {
        received_data = NEW(struct udp_data);

        str_length = tools_str_length(buffer);

        received_data->remote = NEW(struct sockaddr_in6);
        received_data->payload = NEW_ARRAY(char, str_length);

        tools_memcpy(received_data->remote, &remote, sizeof(remote));
        tools_memcpy(received_data->payload, buffer, sizeof(char) * str_length);

#ifdef DEBUG
        inet_ntop(AF_INET6, &remote.sin6_addr, straddr, sizeof(straddr));
        printf("Received packet from %s:%d\nData: %s\n\n", straddr, ntohs(remote.sin6_port), buffer);
#endif

        queue_add(received_data, queue);

        CLEAR(buffer, sizeof(char)*BUFLEN);
    }
    perror("recvfrom");

    return NULL;
}

/* UDP sender thread.
 * It gets data from queue data to send and send it via UDP socket */
void* udp_sender(void *data) {
    int str_length = 0;

#ifdef DEBUG
    char straddr[INET6_ADDRSTRLEN];
#endif

    struct udp_data *send_data = NULL;

    int sockfd = 0;
    struct queue *queue = NULL;

    sockfd = ((struct thread_init*)data)->sockfd;
    queue = ((struct thread_init*)data)->queue;
    DELETE(data);

    while ((send_data = (struct udp_data*)queue_get(queue)) != NULL) {
        str_length = tools_str_length(send_data->payload);

        if (sendto(sockfd, send_data->payload, str_length, 0, (struct sockaddr*)send_data->remote, sizeof(*(send_data->remote))) < 0) {
            perror("sendto");
        }
#ifdef DEBUG
        else {
            inet_ntop(AF_INET6, &(send_data->remote->sin6_addr), straddr, sizeof(straddr));
            printf("Sent packet to %s:%d\nData: %s\n\n", straddr, ntohs(send_data->remote->sin6_port), send_data->payload);
        }
#endif
    }

    return NULL;
}

struct bridge_loopback_init {
    struct queue *received_queue;
    struct queue *send_queue;
};

void* bridge_loopback(void *init) {

#ifdef DEBUG
    char straddr[INET6_ADDRSTRLEN];
#endif

    struct udp_data *data = NULL;

    struct queue *received_queue = NULL, *send_queue = NULL;

    received_queue = ((struct bridge_loopback_init*)init)->received_queue;
    send_queue = ((struct bridge_loopback_init*)init)->send_queue;
    DELETE(data);

    while ((data = (struct udp_data*)queue_get(received_queue)) != NULL) {
#ifdef DEBUG
        inet_ntop(AF_INET6, &(data->remote->sin6_addr), straddr, sizeof(straddr));
        printf("Data from/to %s:%d\nData: %s\n\n", straddr, ntohs(data->remote->sin6_port), data->payload);
#endif
        queue_add(data, send_queue);
    }
    
    return NULL;
}

/* TCP receiver thread. 
 * It receives data from broker and put it to queue to send via UDP socket */
void* tcp_receiver(void *data) {
    int str_length = 0;
    ssize_t result;

    char buffer[BUFLEN];

    struct udp_data *send_data = NULL;

    int sockfd = 0;
    struct queue *queue = NULL;
    struct sockaddr_in6 *remote = NULL;

    sockfd = ((struct thread_init*)data)->sockfd;
    queue = ((struct thread_init*)data)->queue;
    remote = ((struct thread_init*)data)->remote;
    DELETE(data);

    while ((result = recv(sockfd, buffer, BUFLEN, 0)) > 0) {
        send_data = NEW(struct udp_data);

        str_length = tools_str_length(buffer);

        send_data->remote = NEW(struct sockaddr_in6);
        send_data->payload = NEW_ARRAY(char, str_length);

        tools_memcpy(send_data->remote, &remote, sizeof(remote));
        tools_memcpy(send_data->payload, buffer, sizeof(char) * str_length);

        queue_add(send_data, queue);

        CLEAR(buffer, sizeof(char)*BUFLEN);
    }
    perror("recv");

    return NULL;
}

void* tcp_sender(void *data) {

    DELETE(data);
    return NULL;
}

int main() {
    int
        udp_port = 61613, udp_sockfd;

    struct sockaddr_in6
        udp_sockaddr;

    struct thread_init
        *thread_init;

    struct queue
        *udp_receiver_queue, *udp_sender_queue;

    pthread_t
        udp_receiver_thread, udp_sender_thread, bridge_loopback_thread;

    if ((udp_sockfd = socket(PF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    udp_sockaddr.sin6_family = AF_INET6;
    udp_sockaddr.sin6_port = htons(udp_port);
    udp_sockaddr.sin6_addr = in6addr_any;

    if (bind(udp_sockfd, (const struct sockaddr*)(&udp_sockaddr), sizeof(udp_sockaddr)) < 0) {
        perror("bind");
        exit(1);
    }

    udp_receiver_queue = queue_new();
    udp_sender_queue = queue_new();

    thread_init = NEW(struct thread_init);
    thread_init->sockfd = udp_sockfd;
    thread_init->queue = udp_receiver_queue;
    pthread_create(&udp_receiver_thread, NULL, udp_receiver, thread_init);

    thread_init = NEW(struct thread_init);
    thread_init->sockfd = udp_sockfd;
    thread_init->queue = udp_sender_queue;
    pthread_create(&udp_sender_thread, NULL, udp_sender, thread_init);
    
    struct bridge_loopback_init *special_init = NEW(struct bridge_loopback_init);
    special_init->received_queue = udp_receiver_queue;
    special_init->send_queue = udp_sender_queue;
    pthread_create(&bridge_loopback_thread, NULL, bridge_loopback, special_init);
    
    pthread_join(udp_receiver_thread, NULL);
    pthread_join(udp_sender_thread, NULL);
    pthread_join(bridge_loopback_thread, NULL);

    close(udp_sockfd);
    return 0;
}
