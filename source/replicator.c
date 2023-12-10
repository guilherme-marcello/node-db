#include "replicator.h"
#include "utils.h"
#include "table_server.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <zookeeper/zookeeper.h>

typedef struct String_vector zoo_string;

struct ConnectionContext {
    pthread_cond_t* cond;
    pthread_mutex_t* mutex;
    int* connection_established;
};

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



void replicator_init(struct TableServerReplicationData* replicator, struct TableServerOptions* options) {
    if (assert_error(
        replicator == NULL || options == NULL || options->zk_connection_str == NULL,
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

    //zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));

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