
#include "table_server.h"
#include "database.h"
#include "utils.h"
#include "network_server.h"
#include "table_skel.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#ifndef SERVER_GLOBAL_VARIABLES
// ====================================================================================================
//                                        Global Variables
// ====================================================================================================
struct TableServerConfig config;
struct TableServerOptions options;
struct TableServerDatabase database;
#endif

#ifndef SERVER_DATA_STRUCT
// ====================================================================================================
//                                    Server Data Struct (TableServerData)
// ====================================================================================================

void SERVER_INIT(char* argv[]) {
    config.valid = false;
    config.listening_fd = network_server_init(options.listening_port);
    database_init(&database, options.n_lists);

    if (assert_error(
        config.listening_fd < 0 || database.table == NULL,
        "SERVER_INIT",
        "Failed to initialize table server.\n"
    )) return;
    config.valid = true;
}
void SERVER_EXIT(int status) {
    SERVER_FREE();
    exit(status);
}
void SERVER_FREE() {
    assert_error(
        network_server_close(config.listening_fd) == M_ERROR,
        "SERVER_FREE",
        "Failed to free listening file descriptor."
    );
    database_destroy(&database);
}

#endif


#ifndef SERVER_EXECUTION_OPTIONS
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

void parse_args(char* argv[]) { 
    char *endptr;
    int port = strtol(argv[1], &endptr, 10);
    int n = strtol(argv[2], &endptr, 10);

    if (assert_error(
        *endptr != '\0' || port <= 0 || n <= 0,
        "parse_args",
        "Failed to parse arguments. Both port and n must be positive integers."
    )) return;

    options.valid = true;
    options.listening_port = port;
    options.n_lists = n;
    return;
}

void interrupt_handler() {
    SERVER_EXIT(0);
}
#endif

#ifndef TABLE_SERVER_MAIN
// ====================================================================================================
//                                              Main
// ====================================================================================================
int main(int argc, char *argv[]) {
    signal(SIGINT, interrupt_handler);

    // launch usage menu
    usage_menu(argc, argv);
    if (assert_error(
        argc != NUMBER_OF_ARGS,
        "main",
        ERROR_ARGS
    )) return -1;

    parse_args(argv);
    // init server
    SERVER_INIT(argv);
    if (!config.valid)
        SERVER_EXIT(EXIT_FAILURE);

    // Main Loop
    network_main_loop(config.listening_fd, &database);
    SERVER_EXIT(EXIT_FAILURE);
}
#endif