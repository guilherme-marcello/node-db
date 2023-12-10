#include "replicator.h"
#include "utils.h"
#include "table_server.h"
#include "client_stub.h"
#include "database.h"
#include "distributed_database.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <zookeeper/zookeeper.h>

struct ConnectionContext {
    pthread_cond_t* cond;
    pthread_mutex_t* mutex;
    int* connection_established;
};

struct ChildUpdateContext {
    struct TableServerReplicationData* replicator;
    struct TableServerDistributedDatabase* ddb;
};

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
            // if there was a defined next server, check if we should change it
            if (context->replicator->next_server_node_path != NULL) {
                if (next_node == NULL) { // next_node is null => this server is the tail! disconnect current table
                    rtable_disconnect(context->ddb->replica);
                    context->ddb->replica = NULL;
                } else if (string_compare(context->replicator->next_server_node_path, next_node) != EQUAL) {
                    // if is not equal => we should change the next server!
                    rtable_disconnect(context->ddb->replica); // disconnect current next server
                    context->ddb->replica = replicator_get_table(context->replicator, next_node);
                }

            }
            else { // current next server is not defined!
                if (next_node != NULL) {
                    context->ddb->replica = replicator_get_table(context->replicator, next_node);
                }
            }

            printf("Next server change: (%s) -> (%s)\n", context->replicator->next_server_node_path, next_node);
            if (context->replicator->next_server_node_path != NULL) {
                destroy_dynamic_memory(context->replicator->next_server_node_path);
            }
            context->replicator->next_server_node_path = next_node;
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

    printf("Server node created! Node path: %s with data = %s\n", generated_path, node_data);

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

    printf("Node data for %s is %s!\n", path, node_data);
    struct rtable_t* table = rtable_connect(node_data);
    destroy_dynamic_memory(node_data);
    return table;    
}



void replicator_init(struct TableServerReplicationData* replicator, struct TableServerDistributedDatabase* ddb, struct TableServerOptions* options) {
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
    replicator->server_node_path = replicator_create_node(replicator->zh, "127.0.0.1", options->listening_port);
    if (replicator->server_node_path == NULL)
        return;
    
    // 4. watch /chain children
    struct ChildUpdateContext* watch_context = create_dynamic_memory(sizeof(struct ChildUpdateContext));
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
    char* prev_server_node_path = replicator_prev_node(children_list, CHAIN_PATH, replicator->server_node_path);
    printf("Prev server is %s\n", prev_server_node_path);
    if (prev_server_node_path != NULL) {
        printf("Setting up merge from %s to %s\n", prev_server_node_path, replicator->server_node_path);
        struct rtable_t* migration_table = replicator_get_table(replicator, prev_server_node_path);
        if (migration_table != NULL) {
            db_migrate_table(ddb->db, migration_table);
            rtable_disconnect(migration_table);
            destroy_dynamic_memory(prev_server_node_path);
        }       
    }

    // free list
    for (int j = 0; j < children_list->count; j++) {
        destroy_dynamic_memory(children_list->data[j]);
    }
    destroy_dynamic_memory(children_list->data);
    destroy_dynamic_memory(children_list);

    replicator->valid = 1;
    printf("Successfully initialized Replicator!\n");
}

void replicator_destroy(struct TableServerReplicationData* replicator) {
    if (assert_error(
        replicator == NULL,
        "replicator_destroy",
        ERROR_NULL_POINTER_REFERENCE
    )) return;    

    zookeeper_close(replicator->zh);
}