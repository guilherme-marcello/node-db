#ifndef _ZK_SERVER_H
#define _ZK_SERVER_H

#include "table_server.h"
#include "database.h"
#include "distributed_database.h"

#include <zookeeper/zookeeper.h>

// Represents the replication data for a table server
struct TableServerReplicationData {
    zhandle_t* zh;                          // ZooKeeper handle
    char* server_node_path;                 // Path of the server node
    char* next_server_node_path;            // Path of the next server node
    struct TableServerDistributedDatabase* ddb; // datatabse
    int valid;                              // Flag indicating validity
};

/**
 * @brief Handles the change of the next server in replication.
 * 
 * @param replicator The replication data for the table server.
 * @param next_node The path of the next server node.
 */
void handle_next_server_change(struct TableServerReplicationData* replicator, char* next_node);

/**
 * @brief Watches the children of the ZooKeeper node and triggers updates on changes.
 * 
 * @param wzh The handle to the ZooKeeper connection.
 * @param type The type of event.
 * @param state The state of the connection.
 * @param zpath The path of the ZooKeeper node.
 * @param watcher_ctx The context for the child watcher.
 */
void zk_server_child_watcher(zhandle_t* wzh, int type, int state, const char* zpath, void* watcher_ctx);

/**
 * @brief Initializes the ZooKeeper server with replication settings.
 * 
 * @param replicator The replication data for the table server.
 * @param ddb The distributed database associated with the table server.
 * @param options The options for the table server.
 */
void zk_server_init(struct TableServerReplicationData* replicator, struct TableServerDistributedDatabase* ddb, struct TableServerOptions* options);

/**
 * @brief Destroys the resources associated with the ZooKeeper server.
 * 
 * @param replicator The replication data for the table server.
 */
void zk_server_destroy(struct TableServerReplicationData* replicator);

#endif