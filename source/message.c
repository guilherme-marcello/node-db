#include "message.h"
#include "utils.h"

#include <unistd.h>
#include <errno.h>


ssize_t write_all(int sock, const void *buf, size_t n) {
    size_t bytes_written = 0;
    size_t bytes_to_write;
    const void *write_pointer;

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

ssize_t read_all(int sock, void *buf, size_t n) {
    size_t bytes_read = 0;
    size_t bytes_to_read;
    void* read_pointer;

    while (bytes_read < n) {
        bytes_to_read = n - bytes_read;
        read_pointer = buf + bytes_read;

        ssize_t res = read(sock, read_pointer, bytes_to_read);
        if (res < 0) {
            if (errno == EINTR) 
                continue;
            else {
                assert_error(
                    1,
                    "read_n_from_sock",
                    "Failed to read from the socket."
                );
                return -1;
            }
        }
        
        if (res == 0)
            break;

        bytes_read += res;
    }

    return bytes_read;
}