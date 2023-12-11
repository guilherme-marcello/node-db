#ifndef _TABLE_SERVER_H
#define _TABLE_SERVER_H

#include "table.h"

struct TableServerConfig {
    int listening_fd;
    int valid;
};

struct TableServerOptions {
    int listening_port;
    int n_lists;
    char* zk_connection_str;
    int valid;
};

void SERVER_INIT();
void SERVER_EXIT(int status);
void SERVER_FREE();

void ts_interrupt_handler();

// Function to parse argv, updating global TableServerOptions struct
void ts_parse_args(char* argv[]);

// Function to display the information in the given TableServerOptions struct
void ts_show_options(struct TableServerOptions* options);
// ====================================================================================================
//                                          ERROR HANDLING
// ====================================================================================================
#define TS_ERROR_ARGS "\033[0;31m[!] Error:\033[0m Number of arguments is should be 3. Execute `table-server -h` for 'help'.\n"

// Program arguments-related constants
#define TS_NUMBER_OF_ARGS 4
#define TS_USAGE_STR   "\033[1mUsage:\033[0m \033[33m./table-server\033[0m \033[32mport n_list zk_host:zk_port\033[0m\n"\
                    "\033[1mOptions:\033[0m\n"\
                    "  \033[32m-h\033[0m: Print this usage message\n"

#endif