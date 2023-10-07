#include "serialization.h"
#include "serialization-private.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int keyArray_to_buffer(char **keys, char **keys_buf) {
    if (assert_error(
        keys == NULL || keys_buf == NULL,
        "keyArray_to_buffer",
        ERROR_NULL_POINTER_REFERENCE
    )) return M_ERROR;

    // init number of keys and the total size of the buffer.
    int nkeys = 0;
    int total_size = sizeof(int);

    // calculate the total size and count the keys.
    for (int i = 0; keys[i] != NULL; i++) {
        nkeys++;
        total_size += strlen(keys[i]) + 1;
    }

    // allocate memory for the buffer to store the serialized data.
    *keys_buf = create_dynamic_memory(total_size);
    char* write_pointer = *keys_buf;
    if (assert_error(
        *keys_buf == NULL,
        "keyArray_to_buffer",
        ERROR_MALLOC
    )) return M_ERROR;

    // serialize the number of keys and copy it to the buffer (network byte order).
    int serialized_nkeys = htonl(nkeys);
    memcpy(write_pointer, &serialized_nkeys, sizeof(int));
    write_pointer += sizeof(int);

    // copy each key to the buffer
    for (int i = 0; keys[i] != NULL; i++) {
        strcpy(write_pointer, keys[i]);
        write_pointer += strlen(keys[i]) + 1;
    }

    // return buffer size
    return total_size;
}

char **buffer_to_keyArray(char *keys_buf) {
    if (assert_error(
        keys_buf == NULL,
        "buffer_to_keyArray",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;
    char* read_pointer = keys_buf;

    // deserialize the number of keys from the buffer (convert from network byte order).
    int nkeys;
    memcpy(&nkeys, read_pointer, sizeof(int));
    nkeys = ntohl(nkeys);

    // validate nkeys
    if (assert_error(
        nkeys < 0,
        "buffer_to_keyArray",
        ERROR_NKEYS_DES
    )) return NULL;
    read_pointer += sizeof(int);

    // allocate memory for an array of strings to store the deserialized keys.
    char **array = create_dynamic_memory(sizeof(char*) * (nkeys + 1));
    if (assert_error(
        array == NULL,
        "buffer_to_keyArray",
        ERROR_MALLOC
    )) return NULL;

    // deserialize and copy each key from the buffer to the array.
    for (int i = 0; i < nkeys; i++) {
        int key_length = strlen(read_pointer) + 1;
        array[i] = create_dynamic_memory(key_length);
        if (assert_error(
            array[i] == NULL,
            "buffer_to_keyArray",
            ERROR_MALLOC
        )) {
            // clean up allocated memory and return NULL.
            for (int j = i - 1; j >= 0; j--)
                destroy_dynamic_memory(array[j]);
            destroy_dynamic_memory(array);
            return NULL;
        }

        // copy the deserialized key from the buffer to the array.
        strcpy(array[i], read_pointer);
        read_pointer += key_length;
    }

    // return deserialized keys.
    return array;
}