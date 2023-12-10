#ifndef _REPLICATOR_H
#define _REPLICATOR_H /* Módulo para suporte a replicação */

#include "table_server.h"
#include "database.h"
#include "distributed_database.h"

#include <zookeeper/zookeeper.h>

#define CHAIN_PATH "/chain"
#define NODE_PATH "/chain/node"

typedef struct String_vector zoo_string;

struct TableServerReplicationData {
    zhandle_t* zh;
    char* server_node_path;
    char* next_server_node_path;
    int valid;
};

zhandle_t* connect_to_zookeeper(char* zk_connection_str);
int replicator_create_chain_if_none(zhandle_t* zh, const char* path);
void handle_next_server_change(struct TableServerReplicationData* replicator, struct TableServerDistributedDatabase* ddb, char* next_node);
struct rtable_t* replicator_get_table(struct TableServerReplicationData* replicator, const char* path);
char* replicator_create_node(zhandle_t* zh, char* host_str, int host_port);
char* replicator_prev_node(zoo_string* children_list, const char* path, char* child);
char* replicator_next_node(zoo_string* children_list, const char* path, char* child);

void replicator_init(struct TableServerReplicationData* replicator, struct TableServerDistributedDatabase* ddb, struct TableServerOptions* options);
void replicator_destroy(struct TableServerReplicationData* replicator);

#endif