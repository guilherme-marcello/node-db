#include "zk_client.h"

#include "zk_utils.h"
#include "utils.h"

#include <zookeeper/zookeeper.h>

void handle_head_change(struct TableClientReplicationData* replicator, char* head) {
    if (assert_error(
        replicator == NULL || replicator->client == NULL,
        "handle_head_change",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    char* current_head = replicator->head_node_path;
    if (string_compare(current_head, head) != EQUAL) {
        // head changed!
        if (replicator->client->head_table != NULL)
            rtable_disconnect(replicator->client->head_table);
        replicator->client->head_table = head ? zk_table_connect(replicator->zh, head) : NULL;
        replicator->head_node_path = head;
        printf("[ \033[1;34mFault Tolerance\033[0m ] - Write server changed: (\033[1;36m%s\033[0m) -> (\033[1;36m%s\033[0m)\n", current_head, head);
    }
}

void handle_tail_change(struct TableClientReplicationData* replicator, char* tail) {
    if (assert_error(
        replicator == NULL || replicator->client == NULL,
        "handle_tail_change",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    char* current_tail = replicator->tail_node_path;
    if (string_compare(current_tail, tail) != EQUAL) {
        // tail changed!
        if (replicator->client->tail_table != NULL)
            rtable_disconnect(replicator->client->tail_table);
        replicator->client->tail_table = tail ? zk_table_connect(replicator->zh, tail) : NULL;
        replicator->tail_node_path = tail;
        printf("[ \033[1;34mFault Tolerance\033[0m ] - Read server changed: (\033[1;36m%s\033[0m) -> (\033[1;36m%s\033[0m)\n", current_tail, tail);
    }
}

void client_child_watcher(zhandle_t* wzh, int type, int state, const char* zpath, void* watcher_ctx) {
    struct TableClientReplicationData* replicator = (struct TableClientReplicationData*)watcher_ctx;

    // alloc mem for children buffer
    zoo_string* children_list = (zoo_string *)create_dynamic_memory(sizeof(zoo_string));
    if (assert_error(
        children_list == NULL,
        "child_watcher",
        ERROR_MALLOC
    )) return;

    if (state == ZOO_CONNECTED_STATE) {
        if (type == ZOO_CHILD_EVENT) {
            /* Get the updated children and reset the watch */ 
            if (assert_error(
                zoo_wget_children(
                    wzh, 
                    CHAIN_PATH, 
                    client_child_watcher, 
                    watcher_ctx, 
                    children_list) != ZOK,
                "child_watcher",
                "Error setting watch\n"
            )) return;

            // get head and tail server
            char* new_head = zk_get_first_child(children_list, CHAIN_PATH);
            char* new_tail = zk_get_last_child(children_list, CHAIN_PATH);
            handle_head_change(replicator, new_head);
            handle_tail_change(replicator, new_tail);
        }
    }
    destroy_dynamic_memory(children_list);
}

void zk_client_init(struct TableClientReplicationData* replicator, struct TableClientData* client, struct TableClientOptions* options) {
    if (assert_error(
        replicator == NULL || client == NULL || options == NULL,
        "zk_client_init",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    replicator->client = client;

    // 1. retrieve the token
    replicator->zh = zk_connect(options->zk_connection_str);
    if (assert_error(
        replicator->zh == NULL,
        "zk_client_init",
        "Failed to retrieve token from Zookeeper.\n"
    )) return;

    // 2. watch /chain children
    zoo_string* children_list = (zoo_string *)create_dynamic_memory(sizeof(zoo_string));
    if (assert_error(
        children_list == NULL,
        "replicator_init",
        ERROR_MALLOC
    )) return;

    if (zoo_wget_children(replicator->zh, CHAIN_PATH, client_child_watcher, replicator, children_list) != ZOK) {
        fprintf(stderr, "Error setting watch at %s!\n", CHAIN_PATH);
    }

    // get head and tail server
    char* new_head = zk_get_first_child(children_list, CHAIN_PATH);
    if (new_head)
        handle_head_change(replicator, new_head);
    
    char* new_tail = zk_get_last_child(children_list, CHAIN_PATH);
    if (new_tail)
        handle_tail_change(replicator, new_tail);
}