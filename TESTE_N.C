#include "NUCLEO.H"
#include "SYSTEM.H"
#include <stdio.h>
#include <stdlib.h>

extern PTR_DESC dMain, dEsc;
extern PROCESS_LIST *process_list;

/* Função genérica que vários processos vão rodar  */
void far processo_impressao() {
    int i, j;
    for(i = 0; i < 50; i++) {
       
        printf("Executando : %s | Iteracao %d\n", running_bcp->name, i);
        
        /* Um delay apenas para dar tempo do timer interromper e trocar de contexto */
        for(j = 0; j < 10000; j++); 
    }
    
    printf("O %s TERMINOU\n", running_bcp->name);
    
    /* Define o status como finished para o escalonador remover  */
    running_bcp->status = finished; 
    while(1); 
}

int main() {
    /* Inicialização da lista de processos baseada */
    process_list = (PROCESS_LIST *)malloc(sizeof(PROCESS_LIST));
    if(process_list == NULL){
        printf("Erro ao alocar process_list \n");
        return 1;
    }
    initilize_process_list(process_list);

    /* Inicialização dos descritores principais*/
    dMain = cria_desc();
    dEsc = cria_desc();
    newprocess(escalonator, dEsc);

    /* Cria N processos simultâneos apontando para a mesma função */
    create_process("Processo_1", processo_impressao);
    create_process("Processo_2", processo_impressao);
    create_process("Processo_3", processo_impressao);
    create_process("Processo_4", processo_impressao);

    printf("Iniciando o Escalonador \n");
    
    /* Transfere a execução para o escalonador */
    transfer(dMain, dEsc);

    printf("Todos os processos terminaram. Fim do teste.\n");
    return 0;
}