#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "client_stub.h"

struct rtable_t {
    char *server_address;
    int server_port;
    int sockfd;
};

struct rtable_t* rtable_create(char* address_port);

int rtable_destroy(struct rtable_t *rtable);

#endif