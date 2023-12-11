#include "table_client.h"
#include "client_stub-private.h"
#include "utils.h"
#include "stats.h"
#include "zk_client.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>



#ifndef CLIENT_GLOBAL_VARIABLES
// ====================================================================================================
//                                        Global Variables
// ====================================================================================================
struct TableClientData client; // global client struct
struct TableClientOptions options;
struct TableClientReplicationData replicator;
#endif

#ifndef CLIENT_DATA_STRUCT
// ====================================================================================================
//                                    Server Data Struct (TableServerData)
// ====================================================================================================

void CLIENT_INIT() {
    client.valid = false;
    client.terminate = false;
    zk_client_init(&replicator, &client, &options);
    client.valid = true;
}
void CLIENT_EXIT(int status) {
    CLIENT_FREE();
    exit(status);
}
void CLIENT_FREE() {
    if (client.head_table != NULL) {
        assert_error(
            rtable_disconnect(client.head_table) == M_ERROR,
            "CLIENT_FREE",
            "Failed to disconnect from remote head table."
        );
    }
    if (client.tail_table != NULL) {
        assert_error(
            rtable_disconnect(client.tail_table) == M_ERROR,
            "CLIENT_FREE",
            "Failed to disconnect from remote head table."
        );
    }
}

#endif


#ifndef CLIENT_EXECUTION_OPTIONS
// ====================================================================================================
//                                      Program Execution Options
// ====================================================================================================
void usage_menu(int argc, char** argv) {
    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        // print usage string
        printf(TC_USAGE_STR);
        // exit program
        exit(EXIT_SUCCESS);
    }
}

void tc_parse_args(char* argv[]) {
    if (assert_error(
        argv == NULL || argv[1] == NULL,
        "parse_args",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    options.zk_connection_str = argv[1];
    options.valid = true;
}

void tc_show_options(struct TableClientOptions* options) {
    printf("+-----------------------------------+\n");
    printf("|           Client Options          |\n");
    printf("+-----------------------------------+\n");
    printf("| Zookeeper Conn.:  %-15s |\n", options->zk_connection_str);
    printf("| Valid:                     %-6s |\n", options->valid ? "Yes" : "No");
    printf("+-----------------------------------+\n");
}

void tc_interrupt_handler() {
    CLIENT_EXIT(0);
}

#endif

#ifndef CLIENT_STUB_WRAPPERS
// ====================================================================================================
//                                      Client Stub Wrappers
// ====================================================================================================
int stats() {
    struct statistics_t* stats = rtable_stats(client.tail_table);
    if (stats == NULL)
        return -1;

    stats_show(stats);
    stats_destroy(stats);
    return 0;
}

int gettable() {
    struct entry_t** entries = rtable_get_table(client.tail_table);
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
    char** keys = rtable_get_keys(client.tail_table);
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
    int size = rtable_size(client.tail_table);
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
        rtable_del(client.head_table, key) < 0,
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
    struct data_t* data = rtable_get(client.tail_table, key);
    if (data == NULL) {
        printf("Not found.\n");
        return -1;
    }
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

    void* buf_key = duplicate_memory(key, strlen(key) + 1, "put");
    if (buf_key == NULL) {
        destroy_dynamic_memory(buf_value);
        data_destroy(data);
        return -1;
    }

    struct entry_t* entry = entry_create(buf_key, data);
    if (entry == NULL) {
        data_destroy(data);
        destroy_dynamic_memory(buf_key);
        return -1;
    }

    // send request to server
    if (assert_error(
        rtable_put(client.head_table, entry) == -1,
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
    if (!token)
        return INVALID;
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
    else if (!strcmp(token, "stats"))
        return STATS;
    else if (!strcmp(token, "quit"))
        return QUIT;
    return INVALID;
}



void user_interaction() {
    printf(CLIENT_LOADING_CLI);
    char input[MAX_INPUT_LENGTH]; // user input buffer
    while (!client.terminate) {
        while(client.head_table == NULL) {
            printf(CLIENT_WAITING_FOR_SERVERS);
            sleep(5);
        }
        printf(CLIENT_SHELL, options.zk_connection_str, replicator.head_node_path);
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
        case STATS:
            if (stats() == 0)
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
    signal(SIGINT, tc_interrupt_handler);
    
    // launch usage menu
    usage_menu(argc, argv);
    if (assert_error(
        argc != TC_NUMBER_OF_ARGS,
        "main",
        TC_ERROR_ARGS
    )) CLIENT_EXIT(EXIT_FAILURE);

    tc_parse_args(argv);
    tc_show_options(&options);
    if (!options.valid)
        CLIENT_EXIT(EXIT_FAILURE);
    // init client
    CLIENT_INIT();
    if (!client.valid)
        CLIENT_EXIT(EXIT_FAILURE);

    // launch user interaction menu
    user_interaction();
    CLIENT_EXIT(EXIT_SUCCESS);
}
#endif