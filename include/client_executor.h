#ifndef _CLIENT_EXECUTOR_H
#define _CLIENT_EXECUTOR_H /* Módulo client executor */

#include "distributed_database.h"

struct ClientExecutorArgs {
    int client_socket;
    struct TableServerDistributedDatabase *ddb;
};

/* Função que cria e devolve uma estrutura ClientExecutorArgs com os valores passados */
struct ClientExecutorArgs* client_executor_args_create(int client_socket, struct TableServerDistributedDatabase *ddb);

/* Função que processa um requests de um cliente dado o ClientExecutorArgs passado.
*/
void* thread_process_request(void* _args);

/* Função que cria uma thread para atender um cliente */
void launch_client_executor(int client_socket, struct TableServerDistributedDatabase *ddb);


#endif
