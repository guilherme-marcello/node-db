#include "table_skel.h"
#include "table.h"
#include "utils.h"


struct table_t *table_skel_init(int n_lists) {
    return table_create(n_lists);
}

int table_skel_destroy(struct table_t *table) {
    return table_destroy(table);
}

int invoke(MessageT *msg, struct table_t *table) {
    return 0;
}