#ifndef _NETSERVER_PRIVATE_H
#define _NETSERVER_PRIVATE_H

#include "table.h"

/**
 * Process a request received on the given connection socket. This function
 * receives a request, invokes the processing function, sends a response, and
 * then waits for the next request using recursion.
 *
 * @param connection_socket - The socket descriptor for the client connection.
 * @param table - A pointer to the table where the request will be processed.
 */
void process_request(int connection_socket, struct table_t *table);


#endif