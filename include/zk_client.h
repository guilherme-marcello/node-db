#ifndef _ZK_CLIENT_H
#define _ZK_CLIENT_H

#include "table_client.h"

#include <zookeeper/zookeeper.h>

// Represents the replication data for a table client
struct TableClientReplicationData {
    zhandle_t* zh;                          // ZooKeeper handle
    char* head_node_path;                   // Path of the head node
    char* tail_node_path;                   // Path of the tail node
    struct TableClientData* client;         // client data
    int valid;                              // Flag indicating validity
};

/**
 * @brief Initializes the ZooKeeper client with replication settings.
 * 
 * @param replicator The replication data for the table client.
 * @param client The table client associated with the replication.
 * @param options The options for the table client.
 */
void zk_client_init(struct TableClientReplicationData* replicator, struct TableClientData* client, struct TableClientOptions* options);

/**
 * @brief Handles the change of the head node in replication.
 * 
 * @param replicator The replication data for the table client.
 * @param head The path of the new head node.
 */
void handle_head_change(struct TableClientReplicationData* replicator, char* head);

/**
 * @brief Handles the change of the tail node in replication.
 * 
 * @param replicator The replication data for the table client.
 * @param tail The path of the new tail node.
 */
void handle_tail_change(struct TableClientReplicationData* replicator, char* tail);

/**
 * @brief Watches the children of the ZooKeeper node and triggers updates on changes.
 * 
 * @param wzh The handle to the ZooKeeper connection.
 * @param type The type of event.
 * @param state The state of the connection.
 * @param zpath The path of the ZooKeeper node.
 * @param watcher_ctx The context for the child watcher.
 */
void client_child_watcher(zhandle_t* wzh, int type, int state, const char* zpath, void* watcher_ctx);

#endif
