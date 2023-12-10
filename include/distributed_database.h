#ifndef _DDATABASE_H
#define _DDATABASE_H /* Módulo Distributed Database */

#include "database.h"
#include "client_stub.h"

struct TableServerDistributedDatabase {
    struct TableServerDatabase* db;
    struct rtable_t* replica; // remote table to receive forwarded requests
};

void ddatabase_init(struct TableServerDistributedDatabase* db, int n_lists);
void ddatabase_destroy(struct TableServerDistributedDatabase* db);


int ddb_table_put(struct TableServerDistributedDatabase* ddb, char *key, struct data_t *value);
struct data_t* ddb_table_get(struct TableServerDistributedDatabase* ddb, char *key);
int ddb_table_remove(struct TableServerDistributedDatabase* ddb, char* key);
int ddb_table_size(struct TableServerDistributedDatabase* ddb);
char** ddb_table_get_keys(struct TableServerDistributedDatabase* ddb);

#endif