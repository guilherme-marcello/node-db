#include "database.h"

#include "table_skel.h"
#include "utils.h"

void database_init(struct TableServerDatabase* db, int n_lists) {
    db->table = table_skel_init(n_lists);
    pthread_mutex_init(&db->active_mutex, NULL);
    pthread_mutex_init(&db->table_mutex, NULL);
    pthread_mutex_init(&db->op_counter_mutex, NULL);
    pthread_mutex_init(&db->computed_time_mutex, NULL);
}

void database_destroy(struct TableServerDatabase* db) {
    assert_error(
        table_skel_destroy(db->table) == M_ERROR,
        "SERVER_FREE",
        "Failed to free server table."
    );

    pthread_mutex_destroy(&db->active_mutex);
    pthread_mutex_destroy(&db->table_mutex);
    pthread_mutex_destroy(&db->op_counter_mutex);
    pthread_mutex_destroy(&db->computed_time_mutex);
}