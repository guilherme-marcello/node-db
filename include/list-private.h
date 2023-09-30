#ifndef _LIST_PRIVATE_H
#define _LIST_PRIVATE_H

#include "entry.h"

struct node_t {
	struct entry_t *entry;
	struct node_t  *next; 
};

struct list_t {
	int size;
	struct node_t *head;
};

/* Função que cria um novo elemento de dados node_t e que inicializa 
 * os dados de acordo com os argumentos recebidos, sem necessidade de
 * reservar memória para os dados.	
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct node_t *node_create(struct entry_t *entry, struct node_t *next); 

/* Função que elimina um node, apontado pelo parâmetro node,
 * libertando toda a memória por ele ocupada.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
enum MemoryOperationStatus node_destroy(struct node_t *node);

// ====================================================================================================
//                                          ERROR HANDLING
// ====================================================================================================
// Error messages
#define ERROR_CREATE_LIST "\033[0;31m[!] Error:\033[0m Failed to create list.\n"
#define ERROR_DESTROY_LIST "\033[0;31m[!] Error:\033[0m Failed to destroy list.\n"
#define ERROR_CREATE_NODE "\033[0;31m[!] Error:\033[0m Failed to create node.\n"
#define ERROR_DESTROY_NIDE "\033[0;31m[!] Error:\033[0m Failed to destroy node.\n"
#endif