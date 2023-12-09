#ifndef _REPLICATOR_H
#define _REPLICATOR_H /* Módulo para suporte a replicação */

#include <zookeeper/zookeeper.h>


struct TableServerReplicationData {
    zhandle_t *zh;
};

void replicator_init(struct TableServerReplicationData* replicator, char* zk_connection_str);
void replicator_destroy(struct TableServerReplicationData* replicator);

#endif