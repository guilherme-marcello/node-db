#ifndef _APTIME_H
#define _APTIME_H /* Módulo aptime */

#include <sys/time.h>

/* Função que recebe timeval start e timeval end, retornando a diferença em micro segundos
*/
long long delta_microsec(struct timeval* start, struct timeval* end);

#endif