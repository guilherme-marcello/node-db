#include "replicator.h"
#include "utils.h"


#include <pthread.h>
#include <zookeeper/zookeeper.h>

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

void replicator_init(struct TableServerReplicationData* replicator, char* zk_connection_str) {
    if (assert_error(
        replicator == NULL || zk_connection_str == NULL,
        "replicator_init",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    pthread_cond_t connection_established_cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t connection_established_mutex = PTHREAD_MUTEX_INITIALIZER;
    int connection_established = 0;

    struct ConnectionContext connection_ctx = {
        .cond = &connection_established_cond,
        .mutex = &connection_established_mutex,
        .connection_established = &connection_established
    };

    replicator->zh = zookeeper_init(zk_connection_str, connection_watcher, 2000, 0, &connection_ctx, 0);
    if (assert_error(
        replicator->zh == NULL,
        "replicator_init",
        "Error connecting to ZooKeeper server"
    )) {
        pthread_mutex_destroy(&connection_established_mutex);
        pthread_cond_destroy(&connection_established_cond);
        return;
    }

    while (!connection_established) {
        if (pthread_cond_wait(&connection_established_cond, &connection_established_mutex) != 0)
            break;

        printf("Connected!\n\n");
    }

    pthread_mutex_destroy(&connection_established_mutex);
    pthread_cond_destroy(&connection_established_cond);
}

void replicator_destroy(struct TableServerReplicationData* replicator) {
    if (assert_error(
        replicator == NULL,
        "replicator_destroy",
        ERROR_NULL_POINTER_REFERENCE
    )) return;    

    zookeeper_close(replicator->zh);
}