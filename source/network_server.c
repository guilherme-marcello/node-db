#include "network_server.h"
#include "table_skel.h"
#include "sdmessage.pb-c.h"
#include "utils.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/socket.h>

int network_server_init(short port) {
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (assert_error(
        fd < 0,
        "network_server_init",
        "Failed to create socket"
    )) return -1;

    if (assert_error(
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0,
        "network_server_init",
        "Failed to set SO_REUSEADDR"
    )) {
        return close_and_return_failure(fd);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET; // ipv4
    addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
    addr.sin_port = htons(port);

    if (assert_error(
        bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0,
        "network_server_init",
        "Failed to bind to the specified port."
    )) {
        return close_and_return_failure(fd);
    }

    if (assert_error(
        listen(fd, 0) < 0,
        "network_server_init",
        "Failed to listen to socket."
    )) {
        return close_and_return_failure(fd);
    }

    return fd;
}

int get_client(int listening_fd) {
    struct sockaddr_in client;
    socklen_t size_client = sizeof((struct sockaddr *)&client);
    return accept(listening_fd, (struct sockaddr *)&client, &size_client);
}


int network_main_loop(int listening_socket, struct table_t *table) {
    while (true) {
        int client_socket = get_client(listening_socket);
        if (client_socket <= 0)
            return M_ERROR;
        
        MessageT* message = network_receive(client_socket);
        if (message == NULL) {
            close(client_socket);
            continue;
        }

        if (invoke(message, table) == M_ERROR) {
            close(client_socket);
            message_t__free_unpacked(message, NULL);
            continue;
        }

        if (network_send(client_socket, message) == M_ERROR) {
            close(client_socket);
            message_t__free_unpacked(message, NULL);
            continue;
        }
        
    }
    return -1;
    
}

MessageT *network_receive(int client_socket) {
    return NULL;
}

int network_send(int client_socket, MessageT *msg) {
    return -1;
}

int network_server_close(int socket) {
    return close(socket);
}