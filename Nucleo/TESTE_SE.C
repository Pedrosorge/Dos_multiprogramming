#include "NUCLEO.H" /* Núclo multiprogramado */
#include "SYSTEM.H" /* Suporte para multiprogramação */
#include <stdio.h>
#include <stdlib.h>

/* Declara a lista de processos e os descritores da main e do escalador no escopo deste arquivo */
extern PTR_DESC dMain, dEsc;
extern PROCESS_LIST *process_list;

/* Declara um semáforo para acesso ao recurso de impressão na tela */
SEMAPHORE *mutex_tela;

void far processo_critico() {
    int i, j;
    for(i = 0; i < 5; i++) {
        P(mutex_tela); /* Entra na região crítica */
        
        /* Nenhum outro processo vai conseguir imprimir enquanto este bloco não terminar */
        printf("Inicio da secao critica \n");
        printf("Processo na CPU : %s\n", running_bcp->name);
        printf("Iteracao atual : %d\n", i);
        printf("Fim da secao\n\n");
        
        V(mutex_tela); /* Sai da região crítica */
        
        for(j = 0; j < 20000; j++); /* Delay para dar chance aos outros */
    }
    
    terminate_process();
}

int main() {

    /* Inicialização da lista de processos */
    process_list = (PROCESS_LIST *)malloc(sizeof(PROCESS_LIST));
    if (process_list == NULL) {
        printf("Erro ao alocar process_list \n");
        return 1;
    }
    initilize_process_list(process_list);

    /* Inicializa o semáforo de exclusão mútua com contagem 1 */
    mutex_tela = (SEMAPHORE *)malloc(sizeof(SEMAPHORE));
    initialize_semaphore(mutex_tela, 1);

    /* Cria os processos que serão escalonados */
    create_process("Esc_A", processo_critico);
    create_process("Esc_B", processo_critico);
    create_process("Esc_C", processo_critico);

    /* Dispara o sistema */
    start_system();
    return 0;
}