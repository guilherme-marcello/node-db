#ifndef _NETSERVER_PRIVATE_H
#define _NETSERVER_PRIVATE_H

#include "table.h"
#include "distributed_database.h"

/**
 * Process a request received on the given connection socket. This function
 * receives a request, invokes the processing function, sends a response, and
 * then waits for the next request using recursion.
 *
 * @param connection_socket - The socket descriptor for the client connection.
 * @param data - A pointer to the server data.
 */
void process_request(int connection_socket, struct TableServerDistributedDatabase* ddb);

// ====================================================================================================
//                                            MESSAGES
// ====================================================================================================

#define SERVER_WAITING_FOR_CONNECTIONS "[ \033[1;32mServer Status\033[0m ] - Server ready, waiting for connections\n"
#define SERVER_FAILED_CONNECTION "[ \033[1;31mError\033[0m ] - Failed to accept client connection\n"
#define SERVER_RECEIVED_REQUEST "[ \033[1;36mInfo\033[0m ] - Request received!\n"
#define SERVER_SENT_MSG_TO_CLIENT "[ \033[1;36mInfo\033[0m ] - Sent response to the client! Waiting for the next request...\n"

#endif