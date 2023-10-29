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
        close_and_return_failure(fd);
    }

    return 0;

}

