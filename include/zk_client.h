#ifndef _ZK_CLIENT_H
#define _ZK_CLIENT_H

#include "table_client.h"

#include <zookeeper/zookeeper.h>

struct ClientChildUpdateContext {
    struct TableClientReplicationData* replicator;
    struct TableClientData* client;
};

struct TableClientReplicationData {
    zhandle_t* zh;
    char* head_node_path;
    char* tail_node_path;
    struct ClientChildUpdateContext* child_update_context;
    int valid;
};


void zk_client_init(struct TableClientReplicationData* replicator, struct TableClientData* client, struct TableClientOptions* options);

void handle_head_change(struct TableClientReplicationData* replicator, struct TableClientData* client, char* head);

void handle_tail_change(struct TableClientReplicationData* replicator, struct TableClientData* client, char* tail);

void client_child_watcher(zhandle_t* wzh, int type, int state, const char* zpath, void* watcher_ctx);


#endif