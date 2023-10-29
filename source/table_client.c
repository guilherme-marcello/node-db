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
        "Failed to initialize table client."
    )) return;
    client.valid = true;
}
void CLIENT_EXIT(int status) {
    CLIENT_FREE();
    exit(status);
}
void CLIENT_FREE() {
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

        if (input[strlen(input) - 1] == '\n')
            input[strlen(input) - 1] = '\0';

        char* token = strtok(input, " ");

        switch (parse_command(token)) {
        case PUT:
            printf("PUT!!\n");
            break;
        case GET:
            printf("GET!!\n");
            break;
        case DEL:
            printf("DEL!!\n");
            break;
        case SIZE:
            printf("SIZE!!\n");
            break;
        case GETKEYS:
            printf("GETKEYS!!\n");
            break;
        case GETTABLE:
            printf("GETTABLE!!\n");
            break;
        case QUIT:
            printf("quit!!\n");
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
    )) return -1;

    // init client
    CLIENT_INIT(argv);
    if (!client.valid)
        CLIENT_EXIT(EXIT_FAILURE);

    // launch user interaction menu
    user_interaction();
    CLIENT_EXIT(EXIT_SUCCESS);
}
#endif