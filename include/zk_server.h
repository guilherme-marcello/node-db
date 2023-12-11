#ifndef _ZK_SERVER_H
#define _ZK_SERVER_H

#include "table_server.h"
#include "database.h"
#include "distributed_database.h"

#include <zookeeper/zookeeper.h>

struct ServerChildUpdateContext {
    struct TableServerReplicationData* replicator;
    struct TableServerDistributedDatabase* ddb;
};

struct TableServerReplicationData {
    zhandle_t* zh;
    char* server_node_path;
    char* next_server_node_path;
    struct ServerChildUpdateContext* child_update_context;
    int valid;
};

void handle_next_server_change(struct TableServerReplicationData* replicator, struct TableServerDistributedDatabase* ddb, char* next_node);


void zk_server_child_watcher(zhandle_t* wzh, int type, int state, const char* zpath, void* watcher_ctx);

void zk_server_init(struct TableServerReplicationData* replicator, struct TableServerDistributedDatabase* ddb, struct TableServerOptions* options);

void zk_server_destroy(struct TableServerReplicationData* replicator);


#endif