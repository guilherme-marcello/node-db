#include "zk_utils.h"

#include "utils.h"

#include "client_stub.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <zookeeper/zookeeper.h>

char* zk_get_first_child(zoo_string* children_list, const char* path) {
    if (assert_error(
        children_list == NULL || path == NULL,
        "zk_get_first_child",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    // sort!
    qsort(children_list->data, children_list->count, sizeof(char*), sort_string_helper);

    char* head_node = NULL;
    if (children_list->count > 0) {
        // construct the full path of the head node
        char* head = children_list->data[0];
        char full_head_path[strlen(path) + 1 + strlen(head) + 1];
        snprintf(full_head_path, sizeof(full_head_path), "%s/%s", path, head);
        head_node = strdup(full_head_path);
    }

    return head_node;
}

char* zk_get_last_child(zoo_string* children_list, const char* path) {
    if (assert_error(
        children_list == NULL || path == NULL,
        "zk_get_last_child",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    // sort!
    qsort(children_list->data, children_list->count, sizeof(char*), sort_string_helper);

    char* tail_node = NULL;
    if (children_list->count > 0) {
        // construct the full path of the tail node
        char* tail = children_list->data[children_list->count - 1];
        char full_tail_path[strlen(path) + 1 + strlen(tail) + 1];
        snprintf(full_tail_path, sizeof(full_tail_path), "%s/%s", path, tail);
        tail_node = strdup(full_tail_path);
    }

    return tail_node;
}


void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
    // parse context to update variables!
    struct ConnectionContext* connection_ctx = (struct ConnectionContext*)context;

    pthread_mutex_lock(connection_ctx->mutex);
    if (type == ZOO_SESSION_EVENT) {
        if (state == ZOO_CONNECTED_STATE) {
            *(connection_ctx->connection_established) = 1;
            pthread_cond_signal(connection_ctx->cond);
        } else {
            *(connection_ctx->connection_established) = 0;
        }
    }
    pthread_mutex_unlock(connection_ctx->mutex);
}

zhandle_t* zk_connect(char* zk_connection_str) {
    if (assert_error(
        zk_connection_str == NULL,
        "zk_connect",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    pthread_cond_t connection_established_cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t connection_established_mutex = PTHREAD_MUTEX_INITIALIZER;
    int connection_established = 0;

    struct ConnectionContext connection_ctx = {
        .cond = &connection_established_cond,
        .mutex = &connection_established_mutex,
        .connection_established = &connection_established
    };

    zhandle_t* zh = zookeeper_init(zk_connection_str, connection_watcher, 2000, 0, &connection_ctx, 0);
    if (assert_error(
        zh == NULL,
        "zk_connect",
        "Error connecting to ZooKeeper server"
    )) {
        pthread_mutex_destroy(&connection_established_mutex);
        pthread_cond_destroy(&connection_established_cond);
        return NULL;
    }

    // just leave when connection is established!
    while (!connection_established) {
        if (pthread_cond_wait(&connection_established_cond, &connection_established_mutex) != 0)
            break;
    }

    pthread_mutex_destroy(&connection_established_mutex);
    pthread_cond_destroy(&connection_established_cond);
    return zh;
}

int ensure_chain_exists(zhandle_t* zh, const char* path) {
    if (zoo_exists(zh, path, 0, NULL) != ZNONODE)
        return 1;

    // create it!
    if (zoo_create(zh, path, NULL, -1, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0) != ZOK)
        return 0;

    // all good
    return 1;
}

char* zk_register_server(zhandle_t* zh, char* host_str, int host_port) {
    printf("[ \033[1;34mServer Setup\033[0m ] - Registering server...\n");
    // alloc mem for the new_path buffer
    char* generated_path = create_dynamic_memory(1024);
    if (assert_error(
        generated_path == NULL,
        "zk_register_server",
        ERROR_MALLOC
    )) return NULL;

    char node_data[32];
    // fill node data with host and port!
    snprintf(node_data, sizeof(node_data), "%s:%d", host_str, host_port);

    // create the ephemeral sequential ZNode
    if (zoo_create(zh, NODE_PATH, node_data, strlen(node_data), &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, generated_path, 1024) != ZOK) {
        destroy_dynamic_memory(generated_path);
        return NULL;
    }
    printf(
        "[ \033[1;34mServer Setup\033[0m ] - Successfully registered the server on Zookeeper:\n   - Path: \033[1;36m%s\033[0m\n   - Data: \033[1;36m%s\033[0m\n", 
        generated_path, node_data
    );


    // return the node path
    return generated_path;
}

char* zk_find_previous_node(zoo_string* children_list, const char* path, char* child) {
    if (assert_error(
        children_list == NULL || path == NULL || child == NULL,
        "zk_find_previous_node",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    // sort!
    qsort(children_list->data, children_list->count, sizeof(char*), sort_string_helper);

    // look after antecessor!
    char* antecessor = NULL;
    for (int i = 0; i < children_list->count; ++i) {
        // complete path of child in order to compare!
        char full_child_path[strlen(path) + 1 + strlen(children_list->data[i]) + 1];
        snprintf(full_child_path, sizeof(full_child_path), "%s/%s", path, children_list->data[i]);
        if (strcmp(child, full_child_path) == 0) {
            // found child => check if there is a antecessor
            if (i - 1 >= 0) {
                // setup buffer for ant. node path
                char full_antecessor_path[strlen(path) + 1 + strlen(children_list->data[i - 1]) + 1];
                snprintf(full_antecessor_path, sizeof(full_antecessor_path), "%s/%s", path, children_list->data[i - 1]);
                antecessor = strdup(full_antecessor_path);
            }

            break; // interrupt search
        }
    }
    return antecessor;
}

char* zk_find_successor_node(zoo_string* children_list, const char* path, char* child) {
    if (assert_error(
        children_list == NULL || path == NULL || child == NULL,
        "zk_find_successor_node",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    // sort!
    qsort(children_list->data, children_list->count, sizeof(char*), sort_string_helper);

    // look after sucessor!
    char* successor = NULL;
    for (int i = 0; i < children_list->count; ++i) {
        // complete path of child in order to compare!
        char full_child_path[strlen(path) + 1 + strlen(children_list->data[i]) + 1];
        snprintf(full_child_path, sizeof(full_child_path), "%s/%s", path, children_list->data[i]);
        if (string_compare(child, full_child_path) == EQUAL) {
            // found child => check if there is a successor
            if (i + 1 < children_list->count) {
                // setup buffer for suc. node path
                char full_sucessor_path[strlen(path) + 1 + strlen(children_list->data[i + 1]) + 1];
                snprintf(full_sucessor_path, sizeof(full_sucessor_path), "%s/%s", path, children_list->data[i + 1]);
                successor = strdup(full_sucessor_path);
            }
            break; // interrupt search
        }
    }
    return successor;
}

struct rtable_t* zk_table_connect(zhandle_t* zh, const char* path) {    
    int size = 32;
    char* node_data = create_dynamic_memory(size * sizeof(char));
    if (zoo_get(zh, path, 0, node_data, &size, NULL) != ZOK)
        return NULL;

    printf("[ \033[1;34mServer Sync\033[0m ] - Establishing remote connection to \033[1;36m%s\033[0m (%s)\n", path, node_data);
    struct rtable_t* table = rtable_connect(node_data);
    destroy_dynamic_memory(node_data);
    return table;    
}

void zk_free_list(zoo_string* list) {
    // free list
    for (int j = 0; j < list->count; j++) {
        destroy_dynamic_memory(list->data[j]);
    }
    destroy_dynamic_memory(list->data);
    destroy_dynamic_memory(list);
}