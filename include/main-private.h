#ifndef _MAIN_PRIVATE_H
#define _MAIN_PRIVATE_H

enum MemoryOperationStatus {
    M_OK = 0,
    M_ERROR = -1
};

enum ComparisonStatus {
    EQUAL = 0,
    LOWER = -1,
    GREATER = 1,
    CMP_ERROR = -2
};

/* Função que compara duas strings e retorna a ordem das mesmas, sendo esta
 * ordem definida pela ordem alfabética do primeiro caractere não-igual.
 * Retorna 0 se as strings forem iguais, -1 se str1 < str2,
 * 1 se str1 > str2 ou -2 em caso de erro.
 */
enum ComparisonStatus string_compare(char* str1, char* str2);

/* Função que liberta uma zona de memória dada pelo apontador, se este nao for nulo
*/
void safe_free(void* ptr);

/* Função que reserva uma zona de memória dinâmica com tamanho indicado
* por size, preenche essa zona de memória com o valor 0, e retorna um 
* apontador para a mesma.
*/
void* create_dynamic_memory(int size);

/* Função que liberta uma zona de memória dada pelo apontador.
*/
void destroy_dynamic_memory(void* ptr);

/* Função que copia size bytes da memoria apontada por from para uma nova zona de memoria */
void* duplicate_memory(void* from, int size, char* snippet_id);

/* Função que verifica se a condicao condition se verifica e,
se for o caso, imprime a mensagem error_msg. 
Indica na excecao o "snippet_id" associado ao erro.
*/
int assert_error(int condition, char* snippet_id, char* error_msg);

// ====================================================================================================
//                                          ERROR HANDLING
// ====================================================================================================
// Error messages
#define ERROR_MALLOC "\033[0;31m[!] Error:\033[0m Failed to allocate dynamic memory.\n"
#define ERROR_MEMCPY "\033[0;31m[!] Error:\033[0m Memcpy operation failed.\n"

#endif