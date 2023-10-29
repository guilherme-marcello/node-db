#include "network_client.h"
#include "client_stub.h"
#include "utils.h"
#include "client_stub-private.h"


#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/socket.h>

int network_connect(struct rtable_t *rtable) {
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(rtable->server_port);
    if (assert_error(
        inet_pton(AF_INET, rtable->server_address, &server_addr.sin_addr) <= 0,
        "rtable_create",
        "Failed to convert IP address.\n"
    )) return -1;

    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (assert_error(
        fd < 0,
        "network_connect",
        "Failed to create socket\n"
    )) return -1;

    if (assert_error(
        connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1,
        "network_connect",
        "Failed to connect to the table server.\n"
    )) {
        network_close(rtable);
        return -1;
    }

    rtable->sockfd = fd;
    return 0;
}

MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg) {
    if (assert_error(
        rtable == NULL || msg == NULL,
        "network_send_receive",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    size_t msg_size = message_t__get_packed_size(msg);
    short msg_size_be = htons(msg_size); // reorder bytes to be

    // allocate buffer with message size
    uint8_t* buffer = (uint8_t*)create_dynamic_memory(msg_size);
    if (assert_error(
        buffer == NULL,
        "network_send_receive",
        ERROR_MALLOC

    )) {
        network_close(rtable);
        return NULL;
    }

    message_t__pack(msg, buffer);

    // send size
    if (assert_error(
        write_n_to_sock(rtable->sockfd, &msg_size_be, sizeof(msg_size_be)) != sizeof(msg_size_be),
        "network_send_receive",
        "Failed to send size of message to server.\n"
    )) {
        destroy_dynamic_memory(buffer);
        network_close(rtable);
        return NULL;
    }

    // send msg buffer
    if (assert_error(
        write_n_to_sock(rtable->sockfd, (void*)buffer, msg_size) != msg_size,
        "network_send_receive",
        "Failed to send message buffer to server.\n"
    )) {
        destroy_dynamic_memory(buffer);
        network_close(rtable);
        return NULL;
    }
    destroy_dynamic_memory(buffer);

    // size and buffer sent... receive response
    msg_size_be = 0;
    if (assert_error(
        read_n_from_sock(rtable->sockfd, &msg_size_be, sizeof(msg_size_be)) != sizeof(msg_size_be),
        "network_send_receive",
        "Failed to receive server response message size.\n"
    )) {
        network_close(rtable);
        return NULL;
    }

    // allocate memory to receive response message
    msg_size = ntohs(msg_size_be);
    buffer = (uint8_t*)create_dynamic_memory(msg_size);
    if (assert_error(
        buffer == NULL,
        "network_send_receive",
        ERROR_MALLOC
    )) {
        network_close(rtable);
        return NULL;
    };

    MessageT *msg_response = message_t__unpack(NULL, msg_size, buffer);
    destroy_dynamic_memory(buffer);
    return msg_response;
}

int network_close(struct rtable_t *rtable) {
    if (assert_error(
        rtable == NULL,
        "network_close",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    close(rtable->sockfd);
    return 0;
}

