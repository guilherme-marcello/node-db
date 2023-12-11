#include "replicator.h"
#include "utils.h"
#include "table_server.h"
#include "client_stub.h"
#include "database.h"
#include "address.h"
#include "distributed_database.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <zookeeper/zookeeper.h>

void handle_next_server_change(struct TableServerReplicationData* replicator, struct TableServerDistributedDatabase* ddb, char* next_node) {
    if (assert_error(
        replicator == NULL || ddb == NULL,
        "handle_next_server_change",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    char* current_next_node = replicator->next_server_node_path;

    if (current_next_node != NULL) {
        // handle changes when the current next server is defined
        if (next_node == NULL) {
            // this server is now the tail! disconnect the current table
            rtable_disconnect(ddb->replica);
            ddb->replica = NULL;
        } else if (string_compare(current_next_node, next_node) != EQUAL) {
            // if not equal, change the next server
            rtable_disconnect(ddb->replica); // disconnect the current next server
            ddb->replica = replicator_get_table(replicator, next_node);
        }
    } else {
        // handle changes when the current next server is not defined
        if (next_node != NULL) {
            ddb->replica = replicator_get_table(replicator, next_node);
        }
    }

    printf("[ \033[1;34mServer Replication\033[0m ] - Next server change: (\033[1;36m%s\033[0m) -> (\033[1;36m%s\033[0m)\n", current_next_node ? current_next_node : "None", next_node ? next_node : "None");

    // clean up memory
    if (current_next_node != NULL)
        destroy_dynamic_memory(replicator->next_server_node_path);
    replicator->next_server_node_path = next_node;
}

void child_watcher(zhandle_t* wzh, int type, int state, const char* zpath, void* watcher_ctx) {
    // parse context to update next server pointer!
    struct ChildUpdateContext* context = (struct ChildUpdateContext*)watcher_ctx;

    // alloc mem for children buffer
    zoo_string* children_list = (zoo_string *)create_dynamic_memory(sizeof(zoo_string));
    if (assert_error(
        children_list == NULL,
        "child_watcher",
        ERROR_MALLOC
    )) return;

    if (state == ZOO_CONNECTED_STATE) {
        if (type == ZOO_CHILD_EVENT) {
            /* Get the updated children and reset the watch */ 
            if (assert_error(
                zoo_wget_children(
                    context->replicator->zh, 
                    CHAIN_PATH, 
                    child_watcher, 
                    watcher_ctx, 
                    children_list) != ZOK,
                "child_watcher",
                "Error setting watch\n"
            )) return;

            // get next server
            char* next_node = replicator_next_node(children_list, CHAIN_PATH, context->replicator->server_node_path);
            handle_next_server_change(context->replicator, context->ddb, next_node);
        }
    }
    destroy_dynamic_memory(children_list);
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

zhandle_t* connect_to_zookeeper(char* zk_connection_str) {
    if (assert_error(
        zk_connection_str == NULL,
        "connect_to_zookeeper",
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
        "connect_to_zookeeper",
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

int replicator_create_chain_if_none(zhandle_t* zh, const char* path) {
    if (zoo_exists(zh, path, 0, NULL) != ZNONODE)
        return 1;

    // create it!
    if (zoo_create(zh, path, NULL, -1, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0) != ZOK)
        return 0;

    // all good
    return 1;
}

char* replicator_create_node(zhandle_t* zh, char* host_str, int host_port) {
    printf("[ \033[1;34mServer Setup\033[0m ] - Registering server...\n");
    // alloc mem for the new_path buffer
    char* generated_path = create_dynamic_memory(1024);
    if (assert_error(
        generated_path == NULL,
        "replicator_create_node",
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
        generated_path, node_data)
    ;


    // return the node path
    return generated_path;
}

char* replicator_prev_node(zoo_string* children_list, const char* path, char* child) {
    if (assert_error(
        children_list == NULL || path == NULL || child == NULL,
        "replicator_prev_node",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

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

char* replicator_next_node(zoo_string* children_list, const char* path, char* child) {
    if (assert_error(
        children_list == NULL || path == NULL || child == NULL,
        "replicator_next_node",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

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

struct rtable_t* replicator_get_table(struct TableServerReplicationData* replicator, const char* path) {    
    int size = 32;
    char* node_data = create_dynamic_memory(size * sizeof(char));
    if (zoo_get(replicator->zh, path, 0, node_data, &size, NULL) != ZOK)
        return NULL;

    printf("[ \033[1;34mServer Sync\033[0m ] - Establishing remote connection to \033[1;36m%s\033[0m (%s)\n", path, node_data);
    struct rtable_t* table = rtable_connect(node_data);
    destroy_dynamic_memory(node_data);
    return table;    
}

void replicator_init(struct TableServerReplicationData* replicator, struct TableServerDistributedDatabase* ddb, struct TableServerOptions* options) {
    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
    if (assert_error(
        replicator == NULL || ddb == NULL || ddb->db == NULL || options == NULL || options->zk_connection_str == NULL,
        "replicator_init",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    // 1. retrieve the token
    replicator->zh = connect_to_zookeeper(options->zk_connection_str);
    if (replicator->zh == NULL)
        return;


    // 2. make sure that root path is created
    if (!replicator_create_chain_if_none(replicator->zh, CHAIN_PATH))
        return;

    // 3. create and get node id for this server!
    char* server_address_str = get_ip_address();
    replicator->server_node_path = replicator_create_node(replicator->zh, server_address_str ? server_address_str : "127.0.0.1", options->listening_port);
    destroy_dynamic_memory(server_address_str);
    if (replicator->server_node_path == NULL)
        return;
    
    // 4. watch /chain children
    struct ChildUpdateContext* watch_context = create_dynamic_memory(sizeof(struct ChildUpdateContext));
    replicator->child_update_context = watch_context;
    watch_context->ddb = ddb;
    watch_context->replicator = replicator;

    zoo_string* children_list = (zoo_string *)create_dynamic_memory(sizeof(zoo_string));
    if (assert_error(
        children_list == NULL,
        "replicator_init",
        ERROR_MALLOC
    )) return;

    if (ZOK != zoo_wget_children(replicator->zh, CHAIN_PATH, child_watcher, watch_context, children_list)) {
        fprintf(stderr, "Error setting watch at %s!\n", CHAIN_PATH);
    }
    
    // 5. retrieve next server from zk and setup remote table
    replicator->next_server_node_path = replicator_next_node(children_list, CHAIN_PATH, replicator->server_node_path);
    if (replicator->next_server_node_path != NULL) {
        printf("Setting up remote table from %s to %s\n", replicator->server_node_path, replicator->next_server_node_path);
    }

    // 6. retrieve prev server from zk and start migration
    printf("[ \033[1;34mServer Sync\033[0m ] - Checking if there is an available server for synchronization...\n");
    char* prev_server_node_path = replicator_prev_node(children_list, CHAIN_PATH, replicator->server_node_path);
    if (prev_server_node_path != NULL) {
        printf("[ \033[1;34mServer Sync\033[0m ] - Setting up synchronization with server \033[1;36m%s\033[0m\n", prev_server_node_path);
        struct rtable_t* migration_table = replicator_get_table(replicator, prev_server_node_path);
        if (migration_table != NULL) {
            printf("[ \033[1;34mServer Sync\033[0m ] - Established temporary remote session to \033[1;36m%s\033[0m. Starting synchronization...\n", prev_server_node_path);
            db_migrate_table(ddb->db, migration_table);
            rtable_disconnect(migration_table);
            destroy_dynamic_memory(prev_server_node_path);
        }
    }
    printf("[ \033[1;34mServer Sync\033[0m ] - Completed synchronization with other servers (if any)\n");

    // free list
    for (int j = 0; j < children_list->count; j++) {
        destroy_dynamic_memory(children_list->data[j]);
    }
    destroy_dynamic_memory(children_list->data);
    destroy_dynamic_memory(children_list);

    replicator->valid = 1;
}

void replicator_destroy(struct TableServerReplicationData* replicator) {
    if (assert_error(
        replicator == NULL,
        "replicator_destroy",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    destroy_dynamic_memory(replicator->server_node_path);
    destroy_dynamic_memory(replicator->next_server_node_path);
    destroy_dynamic_memory(replicator->child_update_context);
    zookeeper_close(replicator->zh);
}