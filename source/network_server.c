#include "network_server.h"
#include "network_server-private.h"
#include "table_skel.h"
#include "sdmessage.pb-c.h"
#include "utils.h"
#include "message.h"
#include "database.h"
#include "distributed_database.h"
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

int network_main_loop(int listening_socket, struct TableServerDistributedDatabase* ddb) {
    signal(SIGPIPE, SIG_IGN);
    printf(SERVER_WAITING_FOR_CONNECTIONS);
    while (true) {
        int client_socket = get_client(listening_socket);
        if (client_socket == -1) {
            printf(SERVER_FAILED_CONNECTION);
            continue;
        }

        launch_client_executor(client_socket, ddb);
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

void process_request(int connection_socket, struct TableServerDistributedDatabase* ddb) {
    MessageT *request = network_receive(connection_socket);

    if (request != NULL) {
        printf(SERVER_RECEIVED_REQUEST);
        // invoke process...
        if (invoke(request, ddb) == -1) {
            message_t__free_unpacked(request, NULL);
        } else {
            // send response...
            if (network_send(connection_socket, request) == -1) {
                message_t__free_unpacked(request, NULL);
            } else {
                printf(SERVER_SENT_MSG_TO_CLIENT);
                message_t__free_unpacked(request, NULL);
                process_request(connection_socket, ddb);  // use recursion to process next request...
            }
        }
    }
}