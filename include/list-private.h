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

/**
 * Função que cria um novo elemento de dados do tipo node_t e inicializa
 * seus dados com base nos argumentos recebidos. Não é necessário reservar
 * memória para os dados. Retorna a nova estrutura ou NULL em caso de erro.
 *
 * @param entry O ponteiro para a entrada de dados a ser associada ao novo nó.
 * @param next  O ponteiro para o próximo nó na lista encadeada.
 * @return      A nova estrutura node_t inicializada ou NULL em caso de erro.
 */
struct node_t *node_create(struct entry_t *entry, struct node_t *next);


/**
 * Função que elimina um nó apontado pelo parâmetro node, liberando toda a
 * memória por ele ocupada. Retorna 0 (OK) ou -1 em caso de erro.
 *
 * @param node O nó da lista encadeada a ser destruído.
 * @return     O status da operação (enum MemoryOperationStatus).
 */
enum MemoryOperationStatus node_destroy(struct node_t *node);


/**
 * Insere um novo nó na lista encadeada em uma posição específica.
 *
 * Esta função insere um novo nó na lista encadeada representada por 'list' na
 * posição entre os nós 'node_prev' e 'node_next'. O novo nó é representado
 * por 'node'. A função retorna o status da operação de inserção.
 *
 * @param list       A lista encadeada onde ocorrerá a inserção.
 * @param node       O nó a ser inserido.
 * @param node_prev  O nó que será o antecessor do novo nó.
 * @param node_next  O nó que será o sucessor do novo nó.
 * @return           O status da operação de inserção (enum AddOperationStatus).
 */
enum AddOperationStatus insert_node(struct list_t* list, struct node_t* node, struct node_t* node_prev, struct node_t* node_next);

/**
 * Insere um novo nó em ordem crescente na lista encadeada.
 *
 * Esta função insere um novo nó na lista encadeada representada por 'list' em
 * ordem crescente com base na comparação dos valores dos nós. O novo nó é
 * representado por 'node'. A função retorna o status da operação de inserção.
 *
 * @param list  A lista encadeada onde ocorrerá a inserção ordenada.
 * @param node  O nó a ser inserido em ordem crescente.
 * @return      O status da operação de inserção (enum AddOperationStatus).
 */
enum AddOperationStatus insert_node_in_ordered_list(struct list_t* list, struct node_t* node);

/**
 * Função auxiliar para inserção ordenada de um nó em uma lista encadeada.
 *
 * Esta função é usada internamente para inserir um novo nó na lista encadeada
 * representada por 'list' em ordem crescente com base na comparação dos valores
 * dos nós. O novo nó é representado por 'node'. A função recursiva recebe o
 * nó atual ('node_current') para efetuar a inserção ordenada.
 *
 * @param list         A lista encadeada onde ocorrerá a inserção ordenada.
 * @param node         O nó a ser inserido em ordem crescente.
 * @param node_prev    O nó que será o antecessor do novo nó.
 * @param node_current O nó atual para comparação e inserção.
 * @return             O status da operação de inserção (enum AddOperationStatus).
 */
enum AddOperationStatus insert_node_in_ordered_list_aux(struct list_t* list, struct node_t* node, struct node_t* node_prev, struct node_t* node_current);

/**
 * Função auxiliar para remoção de um nó em uma lista encadeada.
 *
 * Esta função é usada internamente para remover um nó da lista encadeada
 * representada por 'list'. O nó a ser removido é identificado pelo seu
 * antecessor ('node_prev') e pelo nó atual ('node_current'). A remoção é
 * realizada com base na chave 'key'. A função retorna o status da operação
 * de remoção (enum RemoveOperationStatus).
 *
 * @param list         A lista encadeada onde ocorrerá a remoção.
 * @param node_prev    O nó antecessor ao nó a ser removido.
 * @param node_current O nó a ser removido.
 * @param key          A chave usada para identificar o nó a ser removido.
 * @return             O status da operação de remoção (enum RemoveOperationStatus).
 */
enum RemoveOperationStatus list_remove_aux(struct list_t *list, struct node_t* node_prev, struct node_t* node_current, char* key);

/**
 * Função auxiliar para obtenção de um nó em uma lista encadeada por chave.
 *
 * Esta função é usada internamente para obter um nó da lista encadeada
 * a começar por 'node' com base na chave 'key'. A função retorna um
 * ponteiro para a entrada correspondente ou NULL se a chave não for encontrada.
 *
 * @param node  O nó da lista encadeada onde a busca ocorrerá.
 * @param key   A chave usada para identificar o nó desejado.
 * @return      Um ponteiro para a entrada correspondente ou NULL se não for encontrada.
 */
struct entry_t *list_get_aux(struct node_t *node, char *key);

/**
 * Função auxiliar para obtenção das chaves de nós em uma lista encadeada.
 *
 * Esta função é usada internamente para obter as chaves dos nós da lista
 * encadeada representada por 'node'. As chaves são armazenadas no array 'array'
 * a partir do índice 'index'. A função percorre a lista e preenche o array com
 * as chaves. O tamanho da lista é limitado por 'list_size'. A função retorna
 * o status da operação (enum MemoryOperationStatus).
 *
 * @param node       O nó da lista encadeada onde a busca das chaves ocorrerá.
 * @param array      O array onde as chaves serão armazenadas.
 * @param index      O índice a partir do qual as chaves serão inseridas no array.
 * @param list_size  O tamanho máximo da lista.
 * @return           O status da operação (enum MemoryOperationStatus).
 */
enum MemoryOperationStatus list_get_keys_aux(struct node_t* node, char** array, int index, int list_size);


// ====================================================================================================
//                                          ERROR HANDLING
// ====================================================================================================
// Error messages
#define ERROR_CREATE_LIST	"\033[0;31m[!] Error:\033[0m Failed to create list.\n"
#define ERROR_DESTROY_LIST	"\033[0;31m[!] Error:\033[0m Failed to destroy list.\n"
#define ERROR_EMPTY_LIST	"\033[0;31m[!] Error:\033[0m List is empty.\n"
#define ERROR_CREATE_NODE	"\033[0;31m[!] Error:\033[0m Failed to create node.\n"
#define ERROR_DESTROY_NIDE	"\033[0;31m[!] Error:\033[0m Failed to destroy node.\n"
#endif