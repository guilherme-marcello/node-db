#include "database.h"

#include "table_skel.h"
#include "client_stub.h"
#include "utils.h"
#include "aptime.h"
#include "stats.h"
#include "entry.h"

#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>

void database_init(struct TableServerDatabase* db, int n_lists) {
    if (assert_error(
        db == NULL,
        "database_init",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    if (assert_error(
        n_lists < 0,
        "database_init",
        ERROR_SIZE
    )) return;

    db->table = table_skel_init(n_lists);
    db->stats = stats_create(0, 0, 0);
    pthread_mutex_init(&db->active_mutex, NULL);
    pthread_mutex_init(&db->table_mutex, NULL);
    pthread_mutex_init(&db->op_counter_mutex, NULL);
    pthread_mutex_init(&db->computed_time_mutex, NULL);
    pthread_attr_init(&db->thread_attr);
    // set the thread attribute to detached mode
    if (assert_error(
        pthread_attr_setdetachstate(&db->thread_attr, PTHREAD_CREATE_DETACHED) != 0,
        "database_init",
        "Failed to pthread_attr_setdetachstate\n"
    )) return;

}

void database_destroy(struct TableServerDatabase* db) {
    if (assert_error(
        db == NULL || db->table == NULL || db->stats == NULL,
        "database_destroy",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    assert_error(
        table_skel_destroy(db->table) == M_ERROR,
        "database_destroy",
        "Failed to free server table."
    );

    assert_error(
        stats_destroy(db->stats) == M_ERROR,
        "database_destroy",
        "Failed to free server stats."
    );
    pthread_mutex_destroy(&db->active_mutex);
    pthread_mutex_destroy(&db->table_mutex);
    pthread_mutex_destroy(&db->op_counter_mutex);
    pthread_mutex_destroy(&db->computed_time_mutex);
    pthread_attr_destroy(&db->thread_attr);
}

void db_decrement_active_clients(struct TableServerDatabase* db) {
    if (assert_error(
        db == NULL || db->stats == NULL,
        "db_decrement_active_clients",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    pthread_mutex_lock(&db->active_mutex);
    db->stats->active_clients--;
    pthread_mutex_unlock(&db->active_mutex);
}

void db_increment_active_clients(struct TableServerDatabase* db) {
    if (assert_error(
        db == NULL || db->stats == NULL,
        "db_increment_active_clients",
        ERROR_NULL_POINTER_REFERENCE
    )) return;
    
    pthread_mutex_lock(&db->active_mutex);
    db->stats->active_clients++;
    pthread_mutex_unlock(&db->active_mutex);
}

void db_increment_op_counter(struct TableServerDatabase* db) {
    if (assert_error(
        db == NULL || db->stats == NULL,
        "db_increment_op_counter",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    pthread_mutex_lock(&db->op_counter_mutex);
    db->stats->op_counter++;
    pthread_mutex_unlock(&db->op_counter_mutex);
}

void db_add_to_computed_time(struct TableServerDatabase* db, long long delta) {
    if (assert_error(
        db == NULL,
        "db_add_to_computed_time",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    pthread_mutex_lock(&db->computed_time_mutex);
    db->stats->computed_time_micros += delta;
    pthread_mutex_unlock(&db->computed_time_mutex);
}

int db_table_put(struct TableServerDatabase* db, char *key, struct data_t *value) {
    if (assert_error(
        db == NULL,
        "db_table_put",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    struct timeval start_time, end_time;
    pthread_mutex_lock(&db->table_mutex);
    gettimeofday(&start_time, NULL);
    int result = table_put(db->table, key, value);
    gettimeofday(&end_time, NULL);
    pthread_mutex_unlock(&db->table_mutex);

    // compute time
    long long delta = delta_microsec(&start_time, &end_time);
    db_add_to_computed_time(db, delta);
    return result;
}

struct data_t* db_table_get(struct TableServerDatabase* db, char *key) {
    if (assert_error(
        db == NULL,
        "db_table_get",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    struct timeval start_time, end_time;
    pthread_mutex_lock(&db->table_mutex);
    gettimeofday(&start_time, NULL);
    struct data_t* result = table_get(db->table, key);
    gettimeofday(&end_time, NULL);
    pthread_mutex_unlock(&db->table_mutex);

    // compute time
    long long delta = delta_microsec(&start_time, &end_time);
    db_add_to_computed_time(db, delta);
    return result;
}


int db_table_remove(struct TableServerDatabase* db, char* key) {
    if (assert_error(
        db == NULL,
        "table_remove",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    struct timeval start_time, end_time;
    pthread_mutex_lock(&db->table_mutex);
    gettimeofday(&start_time, NULL);
    int result = table_remove(db->table, key);
    gettimeofday(&end_time, NULL);
    pthread_mutex_unlock(&db->table_mutex);

    // compute time
    long long delta = delta_microsec(&start_time, &end_time);
    db_add_to_computed_time(db, delta);
    return result;
}

int db_table_size(struct TableServerDatabase* db) {
    if (assert_error(
        db == NULL,
        "db_table_size",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    struct timeval start_time, end_time;
    pthread_mutex_lock(&db->table_mutex);
    gettimeofday(&start_time, NULL);
    int result = table_size(db->table);
    gettimeofday(&end_time, NULL);
    pthread_mutex_unlock(&db->table_mutex);

    // compute time
    long long delta = delta_microsec(&start_time, &end_time);
    db_add_to_computed_time(db, delta);
    return result;
}

char** db_table_get_keys(struct TableServerDatabase* db) {
    if (assert_error(
        db == NULL,
        "db_table_get_keys",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    struct timeval start_time, end_time;
    pthread_mutex_lock(&db->table_mutex);
    gettimeofday(&start_time, NULL);
    char** result = table_get_keys(db->table);
    gettimeofday(&end_time, NULL);
    pthread_mutex_unlock(&db->table_mutex);

    // compute time
    long long delta = delta_microsec(&start_time, &end_time);
    db_add_to_computed_time(db, delta);
    return result;
}

int db_migrate_table(struct TableServerDatabase* db, struct rtable_t* migration_table) {
    if (assert_error(
        db == NULL || db->table == NULL || migration_table == NULL,
        "db_migrate_table",
        ERROR_NULL_POINTER_REFERENCE
    )) return 0;

    struct entry_t** entries = rtable_get_table(migration_table);
    if (assert_error(
        entries == NULL,
        "gettable",
        "Failed to retrieve remote table.\n"
    )) return 0;

    // starting with index 0, iterate over entries, updating this server database
    int index = 0;
    struct entry_t* entry;
    while ((entry = entries[index])) {
        printf("<...> Migrating %s : ", entry->key);
        print_data(entry->value->data, entry->value->datasize);
        db_table_put(db, entry->key, entry->value);
        index++;
    }

    rtable_free_entries(entries);
    return 0;
}