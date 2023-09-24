#include "main-private.h"
#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ====================================================================================================
//                                        Memory Handling
// ====================================================================================================
void safe_free(void* ptr) {
    if (ptr) free(ptr);
}

void* create_dynamic_memory(int size) {
    return calloc(1, size);
}

void destroy_dynamic_memory(void* ptr) {
    safe_free(ptr);
}

void* duplicate_memory(void* from, int size, char* snippet_id) {
    void* copy = create_dynamic_memory(size);
    if (assert_error(
        copy == NULL,
        snippet_id,
        ERROR_MALLOC
    )) return NULL;

    if (assert_error(
        memcpy(copy, from, size) == NULL,
        snippet_id,
        ERROR_MEMCPY
    )) {
        destroy_dynamic_memory(copy);
        return NULL;
    }
    return copy;
}


// ====================================================================================================
//                                        Error Handling
// ====================================================================================================
int assert_error(int condition, char* snippet_id, char* error_msg) {
    if (condition)
        fprintf(stderr, "[%s] %s", snippet_id, error_msg);
    return condition;
}


// ====================================================================================================
//                                              Main
// ====================================================================================================
int main(int argc, char *argv[]) {
    printf("Examples of usage\n");

    void* ptr = (void*)malloc(sizeof(int));
    *(int*)ptr = 12;

    struct data_t* data = data_create(sizeof(ptr), ptr);
    struct data_t* copied = data_dup(data);

    *(int*)ptr = 15;

    printf("original -> %d\n", *(int*)data->data);
    printf("copied -> %d\n", *(int*)copied->data);

    data_destroy(data);
    data_destroy(copied);

    return 0;
}