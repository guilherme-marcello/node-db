#ifndef _REPLICATOR_H
#define _REPLICATOR_H /* Módulo para suporte a replicação */

#include "table_server.h"

#include <zookeeper/zookeeper.h>

#define CHAIN_PATH "/chain"
#define NODE_PATH "/chain/node"

struct TableServerReplicationData {
    zhandle_t* zh;
    char* server_node_path;
    char* next_server_node_path;
    int valid;
};

zhandle_t* connect_to_zookeeper(char* zk_connection_str);
int replicator_create_chain_if_none(zhandle_t* zh, const char* path);
char* replicator_create_node(zhandle_t* zh, char* host_str, int host_port);
char* replicator_prev_node(zhandle_t* zh, const char* path, char* child);
char* replicator_next_node(zhandle_t* zh, const char* path, char* child);

void replicator_init(struct TableServerReplicationData* replicator, struct TableServerOptions* options);
void replicator_destroy(struct TableServerReplicationData* replicator);

#endif