#include "network_server.h"
#include "network_server-private.h"
#include "table_skel.h"
#include "sdmessage.pb-c.h"
#include "utils.h"
#include "message.h"
#include "database.h"
#include "client_executor.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <signal.h>
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
    )) return close_and_return_failure(fd);

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

int network_main_loop(int listening_socket, struct TableServerDatabase* db) {
    signal(SIGPIPE, SIG_IGN);
    printf("Server ready, waiting for connections\n");
    while (true) {
        int client_socket = get_client(listening_socket);
        if (client_socket == -1) {
            printf("Failed to accept client connection.\n");
            continue;
        }

        launch_client_executor(client_socket, db);
    }
    return -1;   
}

MessageT *network_receive(int client_socket) {
    return read_message(client_socket);
}

int network_send(int client_socket, MessageT *msg) {
    return send_message(client_socket, msg);
}

int network_server_close(int socket) {
    return close(socket);
}

void process_request(int connection_socket, struct TableServerDatabase* db) {
    MessageT *request = network_receive(connection_socket);

    if (request != NULL) {
        printf("Request received!\n");
        // invoke process...
        if (invoke(request, db) == -1) {
            message_t__free_unpacked(request, NULL);
        } else {
            // send response...
            printf("Sending response....\n");
            if (network_send(connection_socket, request) == -1) {
                message_t__free_unpacked(request, NULL);
            } else {
                printf("Sent response to the client! Waiting for the next request...\n");
                message_t__free_unpacked(request, NULL);
                process_request(connection_socket, db);  // use recursion to process next request...
            }
        }
    }
}