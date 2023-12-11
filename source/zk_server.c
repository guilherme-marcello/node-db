#include "zk_server.h"

#include "zk_utils.h"
#include "utils.h"
#include "database.h"
#include "distributed_database.h"
#include "address.h"

#include <zookeeper/zookeeper.h>

void handle_next_server_change(struct TableServerReplicationData* replicator, char* next_node) {
    if (assert_error(
        replicator == NULL || replicator->ddb == NULL,
        "handle_next_server_change",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    char* current_next_node = replicator->next_server_node_path;

    if (current_next_node != NULL) {
        // handle changes when the current next server is defined
        if (next_node == NULL) {
            // this server is now the tail! disconnect the current table
            rtable_disconnect(replicator->ddb->replica);
            replicator->ddb->replica = NULL;
        } else if (string_compare(current_next_node, next_node) != EQUAL) {
            // if not equal, change the next server
            rtable_disconnect(replicator->ddb->replica); // disconnect the current next server
            replicator->ddb->replica = zk_table_connect(replicator->zh, next_node);
        }
    } else {
        // handle changes when the current next server is not defined
        if (next_node != NULL) {
            replicator->ddb->replica = zk_table_connect(replicator->zh, next_node);
        }
    }

    printf("[ \033[1;34mServer Replication\033[0m ] - Next server change: (\033[1;36m%s\033[0m) -> (\033[1;36m%s\033[0m)\n", current_next_node ? current_next_node : "None", next_node ? next_node : "None");

    // clean up memory
    if (current_next_node != NULL)
        destroy_dynamic_memory(replicator->next_server_node_path);
    replicator->next_server_node_path = next_node;
}


void zk_server_child_watcher(zhandle_t* wzh, int type, int state, const char* zpath, void* watcher_ctx) {
    // parse context to update next server pointer!
    struct TableServerReplicationData* replicator = (struct TableServerReplicationData*)watcher_ctx;

    // alloc mem for children buffer
    zoo_string* children_list = (zoo_string *)create_dynamic_memory(sizeof(zoo_string));
    if (assert_error(
        children_list == NULL,
        "zk_server_child_watcher",
        ERROR_MALLOC
    )) return;

    if (state == ZOO_CONNECTED_STATE) {
        if (type == ZOO_CHILD_EVENT) {
            /* Get the updated children and reset the watch */ 
            if (assert_error(
                zoo_wget_children(
                    wzh, 
                    CHAIN_PATH, 
                    zk_server_child_watcher, 
                    watcher_ctx, 
                    children_list) != ZOK,
                "zk_server_child_watcher",
                "Error setting watch\n"
            )) return;

            // get next server
            char* next_node = zk_find_successor_node(children_list, CHAIN_PATH, replicator->server_node_path);
            handle_next_server_change(replicator, next_node);
        }
    }
    destroy_dynamic_memory(children_list);
}

void zk_server_init(struct TableServerReplicationData* replicator, struct TableServerDistributedDatabase* ddb, struct TableServerOptions* options) {
    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
    if (assert_error(
        replicator == NULL || ddb == NULL || ddb->db == NULL || options == NULL || options->zk_connection_str == NULL,
        "replicator_init",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    replicator->ddb = ddb;

    // 1. retrieve the token
    replicator->zh = zk_connect(options->zk_connection_str);
    if (replicator->zh == NULL)
        return;


    // 2. make sure that root path is created
    if (!ensure_chain_exists(replicator->zh, CHAIN_PATH))
        return;

    // 3. create and get node id for this server!
    char* server_address_str = get_ip_address();
    replicator->server_node_path = zk_register_server(replicator->zh, server_address_str ? server_address_str : "127.0.0.1", options->listening_port);
    destroy_dynamic_memory(server_address_str);
    if (replicator->server_node_path == NULL)
        return;
    
    // 4. watch /chain children
    zoo_string* children_list = (zoo_string *)create_dynamic_memory(sizeof(zoo_string));
    if (assert_error(
        children_list == NULL,
        "zk_server_init",
        ERROR_MALLOC
    )) return;

    if (ZOK != zoo_wget_children(replicator->zh, CHAIN_PATH, zk_server_child_watcher, replicator, children_list)) {
        fprintf(stderr, "Error setting watch at %s!\n", CHAIN_PATH);
    }
    
    // 5. retrieve next server from zk and setup remote table
    replicator->next_server_node_path = zk_find_successor_node(children_list, CHAIN_PATH, replicator->server_node_path);
    if (replicator->next_server_node_path != NULL) {
        printf("Setting up remote table from %s to %s\n", replicator->server_node_path, replicator->next_server_node_path);
    }

    // 6. retrieve prev server from zk and start migration
    printf("[ \033[1;34mServer Sync\033[0m ] - Checking if there is an available server for synchronization...\n");
    char* prev_server_node_path = zk_find_previous_node(children_list, CHAIN_PATH, replicator->server_node_path);
    if (prev_server_node_path != NULL) {
        printf("[ \033[1;34mServer Sync\033[0m ] - Setting up synchronization with server \033[1;36m%s\033[0m\n", prev_server_node_path);
        struct rtable_t* migration_table = zk_table_connect(replicator->zh, prev_server_node_path);
        if (migration_table != NULL) {
            printf("[ \033[1;34mServer Sync\033[0m ] - Established temporary remote session to \033[1;36m%s\033[0m. Starting synchronization...\n", prev_server_node_path);
            db_migrate_table(ddb->db, migration_table);
            rtable_disconnect(migration_table);
            destroy_dynamic_memory(prev_server_node_path);
        }
    }
    printf("[ \033[1;34mServer Sync\033[0m ] - Completed synchronization with other servers (if any)\n");
    // free list
    zk_free_list(children_list);
    replicator->valid = 1;
}


void zk_server_destroy(struct TableServerReplicationData* replicator) {
    if (assert_error(
        replicator == NULL,
        "zk_server_destroy",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    destroy_dynamic_memory(replicator->server_node_path);
    destroy_dynamic_memory(replicator->next_server_node_path);
    zookeeper_close(replicator->zh);
}