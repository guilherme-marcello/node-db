#include "client_stub.h"
#include "network_client.h"
#include "client_stub-private.h"

#include "entry.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


struct rtable_t* rtable_create(char* address_port) {
    // split 
    char *ip_str = strtok(address_port, ":");
    char *port_str = strtok(NULL, ":");
    if (assert_error(
        ip_str == NULL || port_str == NULL,
        "rtable_create",
        "Invalid connection string format. It should be address:port.\n"
    )) return NULL;

    struct rtable_t* table = create_dynamic_memory(sizeof(struct rtable_t));
    if (assert_error(
        table == NULL,
        "rtable_create",
        ERROR_MALLOC
    )) return NULL;

    table->server_address = ip_str;

    table->server_port = atoi(port_str);

    table->sockfd = -1;
    return table;
}

int rtable_destroy(struct rtable_t *rtable) {
    if (assert_error(
        rtable == NULL,
        "rtable_destroy",
        ERROR_NULL_POINTER_REFERENCE
    )) return M_ERROR;

    close(rtable->sockfd);
    safe_free(rtable);
    return M_OK;
}

struct rtable_t *rtable_connect(char *address_port) {
    if (assert_error(
        address_port == NULL,
        "rtable_connect",
        ERROR_NULL_POINTER_REFERENCE   
    )) return NULL;

    struct rtable_t* table = rtable_create(address_port);
    if (assert_error(
        table == NULL,
        "rtable_connect",
        "Failed to connect to remote table.\n"
    )) return NULL;

    if (assert_error(
        network_connect(table) == -1,
        "rtable_connect",
        "Failed to connect to the table server.\n"
    )) {
        rtable_destroy(table);
        return NULL;
    }
    return table;
}

int rtable_disconnect(struct rtable_t *rtable) {
    return rtable_destroy(rtable);
}

int rtable_put(struct rtable_t *rtable, struct entry_t *entry) {
    return 0;
}

struct data_t *rtable_get(struct rtable_t *rtable, char *key) {
    return NULL;
}

int rtable_del(struct rtable_t *rtable, char *key) {
    return 0;
}

int rtable_size(struct rtable_t *rtable) {
    return 0;
}

char **rtable_get_keys(struct rtable_t *rtable) {
    return NULL;
}

void rtable_free_keys(char **keys) {
    return;
}

struct entry_t **rtable_get_table(struct rtable_t *rtable) {
    return NULL;
}

void rtable_free_entries(struct entry_t **entries) {
    return;
}