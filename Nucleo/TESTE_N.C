#include "NUCLEO.H" /* Núclo multiprogramado */
#include "SYSTEM.H" /* Suporte para multiprogramação */
#include <stdio.h>
#include <stdlib.h>

/* Declara a lista de processos e os descritores da main e do escalador no escopo deste arquivo */
extern PTR_DESC dMain, dEsc;
extern PROCESS_LIST *process_list;

/* Função genérica que vários processos vão rodar */
void far processo_impressao() {
    int i, j;
    
    for (i = 0; i < 50; i++) {
        printf("Executando : %s | Iteracao %d\n", running_bcp->name, i);
        
        /* Um delay apenas para dar tempo do timer interromper e trocar de contexto */
        for (j = 0; j < 10000; j++); 

    }
    
    printf("O %s TERMINOU\n", running_bcp->name);
    
    /* Encerra o processo */
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

    /* Cria N processos simultâneos apontando para a mesma função */
    create_process("Processo_1", processo_impressao);
    create_process("Processo_2", processo_impressao);
    create_process("Processo_3", processo_impressao);
    create_process("Processo_4", processo_impressao);

    printf("Iniciando o Escalonador \n");
    
    /* Transfere a execução para o escalonador */
    start_system();

    printf("Todos os processos terminaram. Fim do teste.\n");
    return 0;
    
}
