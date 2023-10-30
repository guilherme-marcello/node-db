#include "network_client.h"
#include "client_stub.h"
#include "utils.h"
#include "client_stub-private.h"
#include "sdmessage.pb-c.h"
#include "message.h"


#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
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

    if (assert_error(
        rtable->sockfd < 0,
        "network_send_receive",
        "Connection to remote server is down.\n"
    )) return NULL;

    if (send_message(rtable->sockfd, msg) < 0)
        return NULL;

    return read_message(rtable->sockfd);
}


int network_close(struct rtable_t *rtable) {
    if (assert_error(
        rtable == NULL,
        "network_close",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    close(rtable->sockfd);
    rtable->sockfd = -1;
    return 0;
}

