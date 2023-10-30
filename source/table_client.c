#include "table_client.h"
#include "client_stub-private.h"
#include "utils.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



#ifndef CLIENT_GLOBAL_VARIABLES
// ====================================================================================================
//                                        Global Variables
// ====================================================================================================
struct TableClientData client; // global client struct
#endif

#ifndef CLIENT_DATA_STRUCT
// ====================================================================================================
//                                    Server Data Struct (TableServerData)
// ====================================================================================================

void CLIENT_INIT(char* argv[]) {
    client.valid = false;
    client.terminate = false;
    client.table = rtable_connect(argv[1]);
    if (assert_error(
        client.table == NULL,
        "CLIENT_INIT",
        "Failed to initialize table client.\n"
    )) return;
    client.valid = true;
}
void CLIENT_EXIT(int status) {
    CLIENT_FREE();
    exit(status);
}
void CLIENT_FREE() {
    if (client.table == NULL)
        return;
    assert_error(
        rtable_disconnect(client.table) == M_ERROR,
        "CLIENT_FREE",
        "Failed to disconnect from remote table."
    );
}

#endif


#ifndef CLIENT_EXECUTION_OPTIONS
// ====================================================================================================
//                                      Program Execution Options
// ====================================================================================================
void usage_menu(int argc, char** argv) {
    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        // print usage string
        printf(USAGE_STR);
        // exit program
        exit(EXIT_SUCCESS);
    }
}

#endif

#ifndef CLIENT_STUB_WRAPPERS
// ====================================================================================================
//                                      Client Stub Wrappers
// ====================================================================================================
int gettable() {
    struct entry_t** entries = rtable_get_table(client.table);
    if (assert_error(
        entries == NULL,
        "gettable",
        "Failed to retrieve remote table.\n"
    )) return -1;

    // starting with index 0, iterate over entries, printing
    int index = 0;
    struct entry_t* entry;
    while ((entry = entries[index])) {
        printf("%s : ", entry->key);
        print_data(entry->value->data, entry->value->datasize);
        index++;
    }

    rtable_free_entries(entries);
    return 0;
}

int getkeys() {
    char** keys = rtable_get_keys(client.table);
    if (assert_error(
        keys == NULL,
        "getkeys",
        "Failed to retrieve keys of remote table.\n"
    )) return -1;

    // starting with index 0, iterate over keys, printing
    int index = 0;
    char* key;
    while ((key = keys[index])) {
        printf("<key> %s\n", key);
        index++;
    }

    rtable_free_keys(keys);
    return 0;
}

int size() {
    int size = rtable_size(client.table);
    if (assert_error(
        size < 0,
        "size",
        "Failed to retrieve size of remote table.\n"
    )) return -1;

    printf("Table size: %d\n", size);
    return 0;
}

int del(char *key) {
    if (assert_error(
        key == NULL,
        "del",
        "Missing args for DEL operation: del <key>.\n"
    )) return -1;

    printf("Deleting key %s...\n", key);
    if (assert_error(
        rtable_del(client.table, key) < 0,
        "del",
        "Failed to delete key from remote table.\n"
    )) return -1;

    return 0;
}

int get(char *key) {
    if (assert_error(
        key == NULL,
        "get",
        "Missing args for GET operation: get <key>.\n"
    )) return -1;

    printf("Getting key %s...\n", key);
    // retrieve data from remote table
    struct data_t* data = rtable_get(client.table, key);

    if (assert_error(
        data == NULL,
        "get",
        "Failed to retrieve key from remote table.\n"
    )) return -1;

    printf("Found data for key %s: ", key);
    print_data(data->data, data->datasize);
    data_destroy(data);
    return 0;
}

int put(char* key, char* value) {
    if (assert_error(
        key == NULL || value == NULL,
        "put",
        "Missing args for PUT operation: put <key> <value>.\n"
    )) return -1;

    void* buf_value = duplicate_memory(value, strlen(value), "put");
    if (buf_value == NULL)
        return -1;

    struct data_t* data = data_create(strlen(value), buf_value);
    if (data == NULL) {
        destroy_dynamic_memory(buf_value);
        return -1;
    }

    void* buf_key = duplicate_memory(key, strlen(key), "put");
    if (buf_key == NULL) {
        data_destroy(data);
        return -1;
    }

    struct entry_t* entry = entry_create(buf_key, data);
    if (entry == NULL) {
        data_destroy(data);
        destroy_dynamic_memory(buf_key);
        return -1;
    }

    // send requst to server
    if (assert_error(
        rtable_put(client.table, entry) == -1,
        "put",
        "Failed to put entry in remote table.\n"
    )) {
        entry_destroy(entry);
        return -1;
    }
    return 0;
}

#endif

#ifndef CLIENT_CLI
// ====================================================================================================
//                                      Command line Interface
// ====================================================================================================

enum CommandType parse_command(char* token) {
    if (!strcmp(token, "put"))
        return PUT;
    else if (!strcmp(token, "get"))
        return GET;
    else if (!strcmp(token, "del"))
        return DEL;
    else if (!strcmp(token, "size"))
        return SIZE;
    else if (!strcmp(token, "getkeys"))
        return GETKEYS;
    else if (!strcmp(token, "gettable"))
        return GETTABLE;
    else if (!strcmp(token, "quit"))
        return QUIT;
    return INVALID;
}



void user_interaction() {
    char input[MAX_INPUT_LENGTH]; // user input buffer
    while (!client.terminate) {
        printf(CLIENT_SHELL, client.table->server_address, client.table->server_port);
        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        char* token = strtok(input, " \n");
        char *key = strtok(NULL, " \n");
        char *value = strtok(NULL, "\n");

        switch (parse_command(token)) {
        case PUT:
            if (put(key, value) == 0)
                printf("Successful operation.\n");
            break;
        case GET:
            if (get(key) == 0)
                printf("Successful operation.\n");
            break;
        case DEL:
            if (del(key) == 0)
                printf("Successful operation.\n");
            break;
        case SIZE:
            if (size() >= 0)
                printf("Successful operation.\n");
            break;
        case GETKEYS:
            if (getkeys() == 0)
                printf("Successful operation.\n");
            break;
        case GETTABLE:
            if (gettable() == 0)
                printf("Successful operation.\n");
            break;
        case QUIT:
            printf(EXIT_MESSAGE);
            client.terminate = 1;
            break;
        default:
            printf("Not a valid option...\n");
            break;
        }
    }
}

#endif

#ifndef TABLE_CLIENT_MAIN
// ====================================================================================================
//                                              Main
// ====================================================================================================
int main(int argc, char *argv[]) {
    // launch usage menu
    usage_menu(argc, argv);
    if (assert_error(
        argc != NUMBER_OF_ARGS,
        "main",
        ERROR_ARGS
    )) CLIENT_EXIT(EXIT_FAILURE);

    // init client
    CLIENT_INIT(argv);
    if (!client.valid)
        CLIENT_EXIT(EXIT_FAILURE);

    // launch user interaction menu
    user_interaction();
    CLIENT_EXIT(EXIT_SUCCESS);
}
#endif