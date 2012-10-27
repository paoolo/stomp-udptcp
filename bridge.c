#include "bridge.h"

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
