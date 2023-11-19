#include "stats.h"
#include "utils.h"

#include <stdio.h>

struct statistics_t* stats_create(int op_counter, int computed_time_micros, int active_clients) {
    struct statistics_t* stats = create_dynamic_memory(sizeof(struct statistics_t));
    if (assert_error(
        stats == NULL,
        "stats_create",
        ERROR_MALLOC
    )) return NULL;

    stats->op_counter = op_counter;
    stats->computed_time_micros = computed_time_micros;
    stats->active_clients = active_clients;
    return stats;
}

int stats_destroy(struct statistics_t* stats) {
    if (assert_error(
        stats == NULL,
        "stats_destroy",
        ERROR_NULL_POINTER_REFERENCE
    )) return M_ERROR;

    destroy_dynamic_memory(stats);
    return M_OK;
}

void stats_show(struct statistics_t* stats) {
    printf(STATS_STR, stats->op_counter, stats->active_clients, stats->computed_time_micros);
}