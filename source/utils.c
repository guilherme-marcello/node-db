#include "utils.h"
#include "data.h"
#include "entry.h"
#include <stdio.h>
#include <stdlib.h>
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
    if (condition && getenv("verbose"))
        fprintf(stderr, "[%s] %s", snippet_id, error_msg);
    return condition;
}