#include "aptime.h"

#include <sys/time.h>

long long delta_microsec(struct timeval* start, struct timeval* end) {
    return ((end->tv_sec - start->tv_sec) * 1000000LL) + (end->tv_usec - start->tv_usec);
}