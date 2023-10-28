#include "utils.h"
#include "data.h"
#include "entry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

enum ComparisonStatus string_compare(char* str1, char* str2) {
    if (str1 == NULL || str2 == NULL)
        return CMP_ERROR;
    
    int cmp_result = strcmp(str1, str2);
    if (cmp_result < 0)
        return LOWER;
    else if (cmp_result > 0)
        return GREATER;
    return EQUAL;
}

// ====================================================================================================
//                                        Memory Handling
// ====================================================================================================
void safe_free(void* ptr) {
    if (ptr) free(ptr);
}

void* create_dynamic_memory(int size) {
    if (assert_error(
        size <= 0,
        "create_dynamic_memory",
        ERROR_SIZE
    )) return NULL;
    return calloc(1, size);
}

void destroy_dynamic_memory(void* ptr) {
    safe_free(ptr);
}

void* duplicate_memory(void* from, int size, char* snippet_id) {
    if (assert_error(
        from == NULL,
        snippet_id,
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    if (assert_error(
        size <= 0,
        snippet_id,
        ERROR_SIZE
    )) return NULL;

    // allocate memory to the copy
    void* copy = create_dynamic_memory(size);
    if (assert_error(
        copy == NULL,
        snippet_id,
        ERROR_MALLOC
    )) return NULL;

    // perform mem copy
    if (assert_error(
        memcpy(copy, from, size) == NULL,
        snippet_id,
        ERROR_MEMCPY
    )) {
        // destroy allocated memory to copy in case of error
        destroy_dynamic_memory(copy);
        return NULL;
    }
    return copy;
}

// ====================================================================================================
//                                          NETWORK
// ====================================================================================================

ssize_t write_n_to_sock(int sock, const void *buf, size_t n) {
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

ssize_t read_n_from_sock(int sock, void *buf, size_t n) {
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

int close_and_return_failure(int fd) {
    close(fd);
    return -1;
}

// ====================================================================================================
//                                        Error Handling
// ====================================================================================================
int assert_error(int condition, char* snippet_id, char* error_msg) {
    // if condition is satisfied and verbose is set to true, print error
    if (condition && getenv("verbose"))
        fprintf(stderr, "[%s] %s", snippet_id, error_msg);
    return condition;
}