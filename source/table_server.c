
#include "table_server.h"
#include "database.h"
#include "distributed_database.h"
#include "zk_server.h"
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
struct TableServerDistributedDatabase ddatabase;
struct TableServerReplicationData replicator;
#endif

#ifndef SERVER_DATA_STRUCT
// ====================================================================================================
//                                    Server Data Struct (TableServerData)
// ====================================================================================================

void SERVER_INIT() {
    config.valid = false;
    config.listening_fd = network_server_init(options.listening_port);
    ddatabase_init(&ddatabase, options.n_lists);
    zk_server_init(&replicator, &ddatabase, &options);

    if (assert_error(
        config.listening_fd < 0 || ddatabase.db == NULL || ddatabase.db->table == NULL || replicator.zh == NULL,
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
    ddatabase_destroy(&ddatabase);
    zk_server_destroy(&replicator);
}

#endif


#ifndef SERVER_EXECUTION_OPTIONS
// ====================================================================================================
//                                      Program Execution Options
// ====================================================================================================
void usage_menu(int argc, char** argv) {
    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        // print usage string
        printf(TS_USAGE_STR);
        // exit program
        exit(EXIT_SUCCESS);
    }
}

void ts_parse_args(char* argv[]) { 
    char *endptr;
    int port = strtol(argv[1], &endptr, 10);
    int n = strtol(argv[2], &endptr, 10);
    char* zk_connection_str = argv[3];

    if (assert_error(
        *endptr != '\0' || port <= 0 || n <= 0 || zk_connection_str == NULL,
        "parse_args",
        "Failed to parse arguments."
    )) return;

    options.valid = true;
    options.listening_port = port;
    options.n_lists = n;
    options.zk_connection_str = zk_connection_str;
    return;
}

void ts_show_options(struct TableServerOptions* options) {
    printf("+-----------------------------------+\n");
    printf("|           Server Options          |\n");
    printf("+-----------------------------------+\n");
    printf("| Listening Port:           %7d |\n", options->listening_port);
    printf("| Number of Lists:          %7d |\n", options->n_lists);
    printf("| Zookeeper Conn.:  %-15s |\n", options->zk_connection_str);
    printf("| Valid:                     %-6s |\n", options->valid ? "Yes" : "No");
    printf("+-----------------------------------+\n");
}

void ts_interrupt_handler() {
    SERVER_EXIT(0);
}
#endif

#ifndef TABLE_SERVER_MAIN
// ====================================================================================================
//                                              Main
// ====================================================================================================
int main(int argc, char *argv[]) {
    signal(SIGINT, ts_interrupt_handler);

    // launch usage menu
    usage_menu(argc, argv);
    if (assert_error(
        argc != TS_NUMBER_OF_ARGS,
        "main",
        TS_ERROR_ARGS
    )) return -1;

    ts_parse_args(argv);
    ts_show_options(&options);
    if (!options.valid)
        SERVER_EXIT(EXIT_FAILURE);
    // init server
    SERVER_INIT();
    if (!config.valid)
        SERVER_EXIT(EXIT_FAILURE);

    // Main Loop
    network_main_loop(config.listening_fd, &ddatabase);
    SERVER_EXIT(EXIT_FAILURE);
}
#endif