#ifndef _MAIN_PRIVATE_H
#define _MAIN_PRIVATE_H


/* Função que verifica se a condicao condition se verifica e,
se for o caso, imprime a mensagem error_msg. 
Indica na excecao o "snippet_id" associado ao erro.
*/
int assert_error(int condition, char* snippet_id, char* error_msg);

#endif