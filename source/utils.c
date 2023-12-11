#include "utils.h"
#include "data.h"
#include "entry.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

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

// Helper function to compare strings for qsort
int sort_string_helper(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void print_data(void* ptr, int size) {
    char str_buffer[size + 1];
    str_buffer[size] = '\0';
    memcpy(&str_buffer, ptr, size);
    // print!
    printf("%s\n", str_buffer);
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

int close_and_return_failure(int fd) {
    close(fd);
    return -1;
}

int get_client(int listening_fd) {
    struct sockaddr_in client;
    socklen_t size_client = sizeof((struct sockaddr *)&client);
    return accept(listening_fd, (struct sockaddr *)&client, &size_client);
}

// ====================================================================================================
//                                        Error Handling
// ====================================================================================================
int assert_error(int condition, char* snippet_id, char* error_msg) {
    // if condition is satisfied and silent is set to true, print error
    if (condition && getenv("verbose"))
        fprintf(stderr, "[%s] %s", snippet_id, error_msg);
    return condition;
}