#ifndef _DATA_PRIVATE_H
#define _DATA_PRIVATE_H

/* Função que liberta a memória alocada pelos conteúdos da estrutura data_t.
 * Retorna 0 (OK) ou -1 (ERROR) em caso de erro.
 */
int data_cleanup(struct data_t *data);

// ====================================================================================================
//                                          ERROR HANDLING
// ====================================================================================================
// Error messages
#define ERROR_CREATE_DATA "\033[0;31m[!] Error:\033[0m Failed to create data.\n"
#define ERROR_DESTROY_DATA "\033[0;31m[!] Error:\033[0m Failed to destroy data block.\n"

#endif