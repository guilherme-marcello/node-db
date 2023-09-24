#include "main-private.h"
#include "data.h"
#include "entry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* create_dynamic_string(const char* str) {
    if (str == NULL)
        return NULL; 

    char* new_str = (char*)malloc(strlen(str) + 1);
    if (assert_error(
        new_str == NULL,
        "create_dynamic_string",
        ERROR_MALLOC
    )) return NULL;

    strcpy(new_str, str);
    return new_str;
}

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

    printf("[%p] data => [%p] data = %d, size = %d\n", data, data->data, *(int*)data->data, data->datasize);
    printf("[%p] copied => [%p] data = %d, size = %d\n", copied, copied->data, *(int*)copied->data, copied->datasize);

    struct entry_t* entry1 = entry_create(
        create_dynamic_string("hello"), data
    );
    struct entry_t* entry2 = entry_create(
        create_dynamic_string("world"), copied
    );

    struct entry_t* copied_entry = entry_dup(entry1);

    printf(
        "[%p] entry1 => [%p] value = %d, [%p] key = %s\n",
        entry1, entry1->value, *(int*)entry1->value->data, &entry1->key, entry1->key
    );

    printf(
        "[%p] entry2 => [%p] value = %d, [%p] key = %s\n",
        entry2, entry2->value, *(int*)entry2->value->data, &entry2->key, entry2->key
    );

    printf(
        "[%p] copied_entry => [%p] value = %d, [%p] key = %s\n",
        copied_entry, copied_entry->value, *(int*)copied_entry->value->data, &copied_entry->key, copied_entry->key
    );


    printf("Comparing entry1[%s] and entry2[%s] == %d\n", entry1->key, entry2->key, entry_compare(entry1, entry2));
    printf("Comparing entry2[%s] and entry1[%s] == %d\n", entry2->key, entry1->key, entry_compare(entry2, entry1));
    printf("Comparing entry1[%s] and copied_entry[%s] == %d\n", entry1->key, copied_entry->key, entry_compare(entry1, copied_entry));

    entry_destroy(entry1);
    entry_destroy(entry2);
    entry_destroy(copied_entry);

    return 0;
}