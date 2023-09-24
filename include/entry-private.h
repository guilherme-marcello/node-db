#ifndef _ENTRY_PRIVATE_H
#define _ENTRY_PRIVATE_H

/* Função que liberta a memória alocada pelos conteúdos da estrutura entry_t.
 * Retorna 0 (OK) ou -1 (ERROR) em caso de erro.
 */
enum MemoryOperationStatus entry_cleanup(struct entry_t* entry);

// ====================================================================================================
//                                          ERROR HANDLING
// ====================================================================================================
// Error messages
#define ERROR_CREATE_ENTRY "\033[0;31m[!] Error:\033[0m Failed to create entry.\n"
#define ERROR_DESTROY_ENTRY "\033[0;31m[!] Error:\033[0m Failed to destroy entry.\n"

#endif