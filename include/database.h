#ifndef _DATABASE_H
#define _DATABASE_H /* Módulo Database */

#include "table.h"
#include <pthread.h>


struct TableServerDatabase {
    struct table_t* table;
    pthread_mutex_t table_mutex;

    int op_counter;
    pthread_mutex_t op_counter_mutex;
    
    int computed_time_micros;
    pthread_mutex_t computed_time_mutex;

    int active_clients;
    pthread_mutex_t active_mutex;
};

void database_init(struct TableServerDatabase* db, int n_lists);
void database_destroy(struct TableServerDatabase* db);

#endif