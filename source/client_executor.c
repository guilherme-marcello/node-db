#include "client_executor.h"

#include "database.h"
#include "distributed_database.h"
#include "network_server-private.h"
#include "utils.h"

#include <stdio.h>


struct ClientExecutorArgs* client_executor_args_create(int client_socket, struct TableServerDistributedDatabase* ddb) {
    struct ClientExecutorArgs* args = create_dynamic_memory(sizeof(struct ClientExecutorArgs));
    if (assert_error(
        args == NULL,
        "client_executor_create",
        ERROR_MALLOC
    )) return NULL;

    args->client_socket = client_socket;
    args->ddb = ddb;
    return args;
}

void* thread_process_request(void* _args) {
    struct ClientExecutorArgs* args = (struct ClientExecutorArgs*)_args;
    struct TableServerDistributedDatabase* ddb = args->ddb;
    int client_socket = args->client_socket;
    destroy_dynamic_memory(args);

    db_increment_active_clients(ddb->db);
    printf("[ \033[1;36mInfo\033[0m ] - Client connection established\n");
    process_request(client_socket, ddb);
    printf("[ \033[1;36mInfo\033[0m ] - Client connection closed\n");
    db_decrement_active_clients(ddb->db);
    close(client_socket);
    pthread_exit(NULL);
}


void launch_client_executor(int client_socket, struct TableServerDistributedDatabase* ddb) {
    struct ClientExecutorArgs* args = client_executor_args_create(client_socket, ddb);
    if (args == NULL) {
        close(client_socket);
        return;
    }

    pthread_t thread;
    int result = pthread_create(&thread, &ddb->db->thread_attr, thread_process_request, (void*)args);
    if (result != 0) {
        destroy_dynamic_memory(args);
        close(client_socket);
        return;
    }
}


