#ifndef _STATS_H
#define _STATS_H /* Módulo stats */

/* Estrutura que define as estatisticas.
 */
struct statistics_t {
    int op_counter;
    long long computed_time_micros;
    int active_clients;
};

/* Função que cria um novo elemento de dados statistics_t e que inicializa 
 * os dados de acordo com os argumentos recebidos, sem necessidade de
 * reservar memória para os dados.	
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct statistics_t* stats_create(int op_counter, long long computed_time_micros, int active_clients); 

/* Função que elimina um bloco de dados, apontado pelo parâmetro data,
 * libertando toda a memória por ele ocupada.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int stats_destroy(struct statistics_t* stats);

/* Função que imprime uma representação textual de statistics_t */
void stats_show(struct statistics_t* stats);

#define STATS_STR "Current total of completed operations: %d\nCurrent amount of clients: %d\nCurrent amount of computation time (micro s): %lld\n"
#endif