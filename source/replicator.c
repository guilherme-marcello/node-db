#include "replicator.h"
#include "utils.h"


#include <pthread.h>
#include <zookeeper/zookeeper.h>

void replicator_init(struct TableServerReplicationData* replicator, char* zk_connection_str) {
    if (assert_error(
        replicator == NULL || zk_connection_str == NULL,
        "replicator_init",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    pthread_cond_t connection_established_cond;
    pthread_mutex_t connection_established_mutex;
    pthread_cond_init(&connection_established_cond, NULL);
    pthread_mutex_init(&connection_established_mutex, NULL);
    pthread_mutex_lock(&connection_established_mutex);
    static int connection_established;

    void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
        pthread_mutex_lock(&connection_established_mutex);
        if (type == ZOO_SESSION_EVENT) {
            if (state == ZOO_CONNECTED_STATE) {
                connection_established = 1;
                pthread_cond_signal(&connection_established_cond);
            } else {
                connection_established = 0;
            }
        }
        pthread_mutex_unlock(&connection_established_mutex);
    }

    replicator->zh = zookeeper_init(zk_connection_str, connection_watcher, 2000, 0, 0, 0);
    if (assert_error(
        replicator->zh == NULL,
        "replicator_init",
        "Error connecting to ZooKeeper server"
    )) return;

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