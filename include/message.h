#ifndef _MESSAGE_H
#define _MESSAGE_H 

#include <unistd.h>


ssize_t write_all(int sock, const void *buf, size_t n);
ssize_t read_all(int sock, void *buf, size_t n);

#endif