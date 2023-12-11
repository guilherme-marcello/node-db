#ifndef _ZK_UTILS_H
#define _ZK_UTILS_H /* Module for replication support */

#include <zookeeper/zookeeper.h>
#include <pthread.h>

#define CHAIN_PATH "/chain"
#define NODE_PATH "/chain/node"

// Represents the context for a connection
struct ConnectionContext {
    pthread_cond_t* cond;           // Condition variable for signaling
    pthread_mutex_t* mutex;         // Mutex for synchronization
    int* connection_established;    // Flag indicating whether the connection is established
};

// Typedef for the zoo_string structure
typedef struct String_vector zoo_string;

/**
 * @brief Establishes a connection to ZooKeeper.
 * 
 * @param zk_connection_str The connection string for ZooKeeper.
 * @return A handle to the ZooKeeper connection.
 */
zhandle_t* zk_connect(char* zk_connection_str);

/**
 * @brief Ensures that the specified ZNode chain exists; creates it if absent.
 * 
 * @param zh The handle to the ZooKeeper connection.
 * @param path The path of the ZNode chain.
 * @return 1 if the chain exists or is successfully created, 0 otherwise.
 */
int ensure_chain_exists(zhandle_t* zh, const char* path);

/**
 * @brief Establishes a connection to a remote table based on the provided ZooKeeper path.
 * 
 * @param zh The handle to the ZooKeeper connection.
 * @param path The path of the remote table in ZooKeeper.
 * @return A handle to the remote table.
 */
struct rtable_t* zk_table_connect(zhandle_t* zh, const char* path);

/**
 * @brief Registers the server in ZooKeeper under the specified path.
 * 
 * @param zh The handle to the ZooKeeper connection.
 * @param host_str The host string.
 * @param host_port The host port.
 * @return The path of the registered server in ZooKeeper.
 */
char* zk_register_server(zhandle_t* zh, char* host_str, int host_port);

/**
 * @brief Finds the previous node in the children list relative to the specified child.
 * 
 * @param children_list The list of children.
 * @param path The path of the parent ZNode.
 * @param child The path of the child ZNode.
 * @return The path of the previous node, or NULL if not found.
 */
char* zk_find_previous_node(zoo_string* children_list, const char* path, char* child);

/**
 * @brief Finds the successor node in the children list relative to the specified child.
 * 
 * @param children_list The list of children.
 * @param path The path of the parent ZNode.
 * @param child The path of the child ZNode.
 * @return The path of the successor node, or NULL if not found.
 */
char* zk_find_successor_node(zoo_string* children_list, const char* path, char* child);

/**
 * @brief Gets the path of the first child in the list.
 * 
 * @param children_list The list of children.
 * @param path The path of the parent ZNode.
 * @return The path of the first child, or NULL if the list is empty.
 */
char* zk_get_first_child(zoo_string* children_list, const char* path);

/**
 * @brief Gets the path of the last child in the list.
 * 
 * @param children_list The list of children.
 * @param path The path of the parent ZNode.
 * @return The path of the last child, or NULL if the list is empty.
 */
char* zk_get_last_child(zoo_string* children_list, const char* path);

/**
 * @brief Free the memory used by the given list
 * 
 * @param list The list
 */
void zk_free_list(zoo_string* list);

#endif
