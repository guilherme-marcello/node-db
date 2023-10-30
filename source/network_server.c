#include "network_server.h"
#include "table_skel.h"
#include "sdmessage.pb-c.h"
#include "utils.h"
#include "message.h"

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

void process_request(int connection_socket, struct table_t *table) {
    MessageT *request = network_receive(connection_socket);

    if (request != NULL) {
        printf("Request received!\n");
        // invoke process...
        if (invoke(request, table) == -1) {
            message_t__free_unpacked(request, NULL);
        } else {
            // send response...
            printf("Sending response....\n");
            if (network_send(connection_socket, request) == -1) {
                message_t__free_unpacked(request, NULL);
            } else {
                printf("Sent response to the client! Waiting for the next request...\n");
                message_t__free_unpacked(request, NULL);
                process_request(connection_socket, table);  // use recursion to process next request...
            }
        }
    }
}


int network_main_loop(int listening_socket, struct table_t *table) {
    signal(SIGPIPE, SIG_IGN);
    while (true) {
        printf("Server ready, waiting for connections\n");
        int client_socket = get_client(listening_socket);
        if (client_socket == -1) {
            printf("Failed to accept client connection.\n");
            continue;
        }

        printf("Client connection established.\n");
        process_request(client_socket, table);
        printf("Client connection closed.\n");
        close(client_socket);
    }
    return -1;
    
}

MessageT *network_receive(int client_socket) {
    // get request message size
    unsigned short msg_size_be;
    if (assert_error(
        read_all(client_socket, &msg_size_be, sizeof(msg_size_be)) != sizeof(msg_size_be),
        "network_receive",
        "Failed to receive client message's size.\n"
    )) return NULL;

    size_t msg_size = ntohs(msg_size_be);

    // allocate memory to receive request message
    void* buffer = (uint8_t*)create_dynamic_memory(msg_size * sizeof(uint8_t*));
    if (assert_error(
        buffer == NULL,
        "network_receive",
        ERROR_MALLOC
    )) return NULL;

    // copy request to buffer..
    if (assert_error(
        read_all(client_socket, buffer, msg_size) != msg_size,
        "network_receive",
        "Failed to read client request.\n"
    )) {
        destroy_dynamic_memory(buffer);
        return NULL;
    }

    // unpack message
    MessageT *msg_request = message_t__unpack(NULL, msg_size, buffer);
    destroy_dynamic_memory(buffer);

    return msg_request;
}

int network_send(int client_socket, MessageT *msg) {
    if (assert_error(
        msg == NULL,
        "network_send",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    size_t msg_size = message_t__get_packed_size(msg);
    unsigned short msg_size_be = htons(msg_size); // reorder bytes to be

    // allocate buffer with message size
    uint8_t* buffer = (uint8_t*)create_dynamic_memory(msg_size * sizeof(uint8_t*));
    if (assert_error(
        buffer == NULL,
        "network_send",
        ERROR_MALLOC
    )) return -1;

    message_t__pack(msg, buffer);

    // send size
    if (assert_error(
        write_all(client_socket, &msg_size_be, sizeof(msg_size_be)) != sizeof(msg_size_be),
        "network_send",
        "Failed to send size of message.\n"
    )) {
        destroy_dynamic_memory(buffer);
        return -1;
    }

    // send msg buffer
    if (assert_error(
        write_all(client_socket, (void*)buffer, msg_size) != msg_size,
        "network_send",
        "Failed to send message buffer.\n"
    )) {
        destroy_dynamic_memory(buffer);
        return -1;
    }

    destroy_dynamic_memory(buffer);
    return 0;
}

int network_server_close(int socket) {
    return close(socket);
}