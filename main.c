#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#include "tools.h"
#include "queue.h"
#include "main.h"

#define DEBUG
#define BUFLEN 512

int map_compare(char *object, char *another) {
    return strcmp(object, another);
}

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

struct bridge_udp_forward_init {
    struct queue *received_queue, send_queue;
    struct map *tcp_connection_map;
}

void* bridge_udp_forward(void *init) {

    char addr[INET6_ADDRSTRLEN];

    struct udp_data *data = NULL;

    struct queue *received_queue = NULL, *send_queue = NULL;
    struct map *tcp_connection_map = NULL;
    struct 

    received_queue = ((struct bridge_udp_forward_init*)init)->received_queue;
    send_queue = ((struct birdge_udp_forward_init*)init)->send_queue;
    tcp_connection_map = ((struct bridge_udp_forward_init*)init)->tcp_connection_map;
    DELETE(data);

    while ((data = (struct udp_data*)queue_get(received_queue)) != NULL) {
        inet_ntop(AF_INET6, &(data->remote->sin6_addr), addr, sizeof(straddr));
#ifdef DEBUG
        printf("Data from %s:%d\nData: %s\n\n", straddr, ntohs(data->remote->sin6_port), data->payload);
#endif
        map_get(addr, tcp_connection_map)
        /* TODO znalezc polaczenie z podanym adresem IPv6 i portem komunikacji po UDP */
        /* Jesli istnieje polaczenie TCP/IPv4, to istnieje tez kolejka do umieszczenia danych do wyslania przez to polaczenie */
        /* Jesli nie istnieje polaczenie TCP/IPv4, to nalezy je utworzyc, czyli: utworzyc kolejke do wysylania danych, */
    }
    
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
    
    struct map
        *tcp_connection_map;

    pthread_t
        udp_receiver_thread, udp_sender_thread, bridge_udp_forward_thread;

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
    
    struct bridge_udp_forward_init *special_init = NEW(struct bridge_udp_forward_init);
    special_init->received_queue = udp_receiver_queue;
    special_init->send_queue = udp_sender_queue;
    special_init->tcp_connection_map = tcp_connection_map;
    pthread_create(&bridge_udp_forward_thread, NULL, bridge_udp_forward, special_init);
    
    pthread_join(udp_receiver_thread, NULL);
    pthread_join(udp_sender_thread, NULL);
    pthread_join(bridge_udp_forward_thread, NULL);

    close(udp_sockfd);
    return 0;
}
