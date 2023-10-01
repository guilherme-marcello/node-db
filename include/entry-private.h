#ifndef _ENTRY_PRIVATE_H
#define _ENTRY_PRIVATE_H

/* Função que liberta a memória alocada pelos conteúdos da estrutura entry_t.
 * Retorna 0 (OK) ou -1 (ERROR) em caso de erro.
 */
enum MemoryOperationStatus entry_cleanup(struct entry_t* entry);

/**
 * Cria uma cópia de uma entrada (entry_t) com uma nova chave e dados.
 *
 * Esta função cria uma nova entrada (entry_t) com base em uma chave (key) e
 * dados (data) fornecidos como argumentos. A função aloca memória para a cópia
 * da chave e dos dados, e cria uma nova entrada que os armazena. É de
 * responsabilidade do chamador liberar a memória da entrada retornada quando
 * não for mais necessária.
 *
 * @param key   A chave para a nova entrada (será copiada).
 * @param data  Os dados para a nova entrada (serão copiados).
 * @return      Um ponteiro para a nova entrada criada, ou NULL em caso de erro
 *              de alocação de memória.
 */
struct entry_t* entry_copy_create(char *key, struct data_t* data);

// ====================================================================================================
//                                          ERROR HANDLING
// ====================================================================================================
// Error messages
#define ERROR_CREATE_ENTRY "\033[0;31m[!] Error:\033[0m Failed to create entry.\n"
#define ERROR_DESTROY_ENTRY "\033[0;31m[!] Error:\033[0m Failed to destroy entry.\n"

#endif