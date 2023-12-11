#ifndef _TABLE_CLIENT_H
#define _TABLE_CLIENT_H

#include "client_stub.h"

#define MAX_INPUT_LENGTH 256

struct TableClientData {
    struct rtable_t* head_table;
    struct rtable_t* tail_table;
    int valid;
    int terminate;
};

struct TableClientOptions {
    char* zk_connection_str;
    int valid;
};

void CLIENT_INIT();
void CLIENT_EXIT(int status);
void CLIENT_FREE();

void tc_interrupt_handler();
// Function to parse argv, updating global TableClientOptions struct
void tc_parse_args(char* argv[]);

// Function to display the information in the given TableClientOptions struct
void tc_show_options(struct TableClientOptions* options);

// ====================================================================================================
//                                      Client Stub Wrappers
// ====================================================================================================
// helpers to perform an action over a remote table
// parsing given values, translating to skel calls over global TableClientData's remote table
// returning 0 if OK, -1 otherwise.
int gettable();
int getkeys();
int size();
int del(char *key);
int get(char *key);
int put(char* key, char* value);

// ====================================================================================================
//                                          ERROR HANDLING
// ====================================================================================================
#define TC_ERROR_ARGS "\033[0;31m[!] Error:\033[0m Number of arguments is should be 2. Execute `table-server -h` for 'help'.\n"

#define CLIENT_SHELL "\033[1;32mtable-client:%s:%d~/\033[0m$ \033[?12;25h"
#define EXIT_MESSAGE "Bye, bye!\n"

// Program arguments-related constants
#define TC_NUMBER_OF_ARGS 2
#define TC_USAGE_STR   "\033[1mUsage:\033[0m \033[33m./table-client\033[0m \033[32mserver:port\033[0m\n"\
                    "\033[1mOptions:\033[0m\n"\
                    "  \033[32m-h\033[0m: Print this usage message\n"

#endif