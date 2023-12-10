#ifndef _REPLICATOR_H
#define _REPLICATOR_H /* Módulo para suporte a replicação */

#include "table_server.h"

#include <zookeeper/zookeeper.h>

#define CHAIN_PATH "/chain"
#define NODE_PATH "/chain/node"

struct TableServerReplicationData {
    zhandle_t* zh;
    char* server_node_path;
    int valid;
};

void replicator_init(struct TableServerReplicationData* replicator, struct TableServerOptions* options);
void replicator_destroy(struct TableServerReplicationData* replicator);

#endif