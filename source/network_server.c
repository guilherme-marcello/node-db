#include "network_server.h"
#include "utils.h"

#include <stdio.h>
#include <unistd.h>
#include <errno>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

ssize_t write_n_to_sock(int sock, const void *buf, size_t n) {
    size_t bytes_written = 0;
    size_t bytes_to_write;
    void *write_pointer;

    while (bytes_written < n) {
        bytes_to_write = n - bytes_written;
        write_pointer = buf + bytes_written;

        ssize_t res = write(sock, write_pointer, bytes_to_write);
        if (res < 0) {
            if (errno == EINTR)
                continue;
            else {
                assert_error(
                    1,
                    "write_n_to_sock",
                    "Failed to write to the socket."
                );
                return -1;
            }
        }

        bytes_written += res;
    }

    return bytes_written;
}

ssize_t read_n_from_sock(int sock, void *buf, size_t n) {
    size_t bytes_read = 0;
    size_t bytes_to_read;
    void* read_pointer;

    while (bytes_read < n) {
        bytes_to_read = n - bytes_read;
        read_pointer = buf + bytes_read;

        ssize_t res = read(sock, read_pointer, bytes_to_read);
        if (res < 0) {
            if (errno==EINTR) 
                continue
            else {
                assert_error(
                    1,
                    "read_n_from_sock",
                    "Failed to read from the socket."
                )
                return -1;
            }
        }
        
        if (res == 0)
            break;

        bytes_read += res;
    }

    return bytes_read;
}

int close_and_return_failure(int fd) {
    close(fd);
    return -1;
}


int network_server_init(short port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
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