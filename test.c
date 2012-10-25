struct udp_data* udp_recvfrom(int sockfd) {

    struct udp_data *data = NULL;
    char *temp = NULL;

    char buffer[BUFLEN];
    int sockfd, result;
    socklen_t length = sizeof(struct sockaddr_in6);

    data = NEW(struct udp_data);
    data->remote = NEW(struct sockaddr_in6);

    while ((result = recvfrom(sockfd, buffer, BUFLEN, 0, (struct sockaddr*)(data->remote), &length)) > 0) {
        temp = tools_str_expand(data->payload, result);
        memcpy(temp + tools_str_length(data->payload), buffer, result);
        DELETE(data->payload);
        data->payload = temp;
        CLEAR(buffer, sizeof(char)*BUFLEN);
        
        if (buffer[result-1] == 0) {
            break;
        }
    }

    return data;
}

struct bridge_forward_client_init {
    void *queue;
}

void* bridge_forward_client(void *data) {

    void *queue = NULL;
    struct udp_received_data *received_data = NULL;

    queue = ((struct bridge_forward_client_init*)data)->queue;
    DELETE(data);

    /*
        1. Odbieramy kolejny komunikat z powiazanym adresem zdalnym i szukamy polaczenia z serwerem, ktore je obsluguje
        1.a jesli brak polaczenia, to zmuszeni jestesmy utworzyc nowe, uruchomic nowe watki do polaczenia TCP i przekazac komunikat do wyslania
        1.b jesli istnieje, to przekazujemy komunikat do odpowiedniej kolejki..
     */

    while ((received_data = /* TODO pobieranie danych z kolejki */) != NULL) {
        if (!/* TODO szukanie polaczenia wg zdalnego adresu */) {
            /* TODO utworzenie nowego polaczenia */
        }
        /* TODO przekazanie komunikatu do kolejki polaczenia */
    }

    DELETE(data);
    return NULL;
}
