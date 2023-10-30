#ifndef _TABLE_CLIENT_H
#define _TABLE_CLIENT_H

#include "client_stub.h"

#define MAX_INPUT_LENGTH 256

struct TableClientData {
    struct rtable_t* table;
    int valid;
    int terminate;
};


void CLIENT_INIT(char* argv[]);
void CLIENT_EXIT(int status);
void CLIENT_FREE();

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
#define ERROR_ARGS "\033[0;31m[!] Error:\033[0m Number of arguments is should be 2. Execute `table-server -h` for 'help'.\n"

#define CLIENT_SHELL "\033[1;32mtable-client:%s:%d~/\033[0m$ \033[?12;25h"
#define EXIT_MESSAGE "Bye, bye!\n"

// Program arguments-related constants
#define NUMBER_OF_ARGS 2
#define USAGE_STR   "\033[1mUsage:\033[0m \033[33m./table-client\033[0m \033[32mserver:port\033[0m\n"\
                    "\033[1mOptions:\033[0m\n"\
                    "  \033[32m-h\033[0m: Print this usage message\n"

#endif