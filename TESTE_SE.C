#include "NUCLEO.H"
#include "SYSTEM.H"
#include <stdio.h>
#include <stdlib.h>

extern PTR_DESC dMain, dEsc;
extern PROCESS_LIST *process_list;

SEMAPHORE *mutex_tela;

void far processo_critico() {
    int i, j;
    for(i = 0; i < 5; i++) {
        P(mutex_tela); // Entra na região crítica 
        
        // Nenhum outro processo vai conseguir imprimir enquanto este bloco não terminar
        printf("Inicio da secao critica \n");
        printf("Processo na CPU : %s\n", running_bcp->name);
        printf("Iteracao atual : %d\n", i);
        printf("Fim da secao\n\n");
        
        V(mutex_tela); // Sai da região crítica 
        
        for(j = 0; j < 20000; j++); // Delay para dar chance aos outros 
    }
    
    running_bcp->status = finished;
    while(1);
}

int main() {
    process_list = (PROCESS_LIST *)malloc(sizeof(PROCESS_LIST));
    initilize_process_list(process_list);

    dMain = cria_desc();
    dEsc = cria_desc();
    newprocess(escalonator, dEsc);

    /* Inicializa o semáforo de exclusão mútua com contagem 1 */
    mutex_tela = (SEMAPHORE *)malloc(sizeof(SEMAPHORE));
    initialize_semaphore(mutex_tela, 1);

    create_process("Esc_A", processo_critico);
    create_process("Esc_B", processo_critico);
    create_process("Esc_C", processo_critico);

    transfer(dMain, dEsc);
    return 0;
}