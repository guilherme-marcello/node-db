#ifndef _ZK_UTILS_H
#define _ZK_UTILS_H /* Módulo para suporte a replicação */

#include <zookeeper/zookeeper.h>

#define CHAIN_PATH "/chain"
#define NODE_PATH "/chain/node"

struct ConnectionContext {
    pthread_cond_t* cond;
    pthread_mutex_t* mutex;
    int* connection_established;
};

typedef struct String_vector zoo_string;

zhandle_t* connect_to_zookeeper(char* zk_connection_str);
int replicator_create_chain_if_none(zhandle_t* zh, const char* path);

struct rtable_t* replicator_get_table(zhandle_t* zh, const char* path);

char* replicator_create_node(zhandle_t* zh, char* host_str, int host_port);

char* replicator_prev_node(zoo_string* children_list, const char* path, char* child);

char* replicator_next_node(zoo_string* children_list, const char* path, char* child);

char* replicator_get_head_node(zoo_string* children_list, const char* path);

char* replicator_get_tail_node(zoo_string* children_list, const char* path);




#endif