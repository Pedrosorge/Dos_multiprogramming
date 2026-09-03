#include "NUCLEO.H"
#include "SYSTEM.H"
#include <stdio.h>

#define TESTE_SIZE 200
#define BUFFER_SIZE 10
#define MAX_RAND 20

extern PTR_DESC dMain, dEsc;
extern PROCESS_LIST *process_list;

SEMAPHORE *mutex, *empty, *full;

FILE *arq;
FILE *log_arq;

int i_buf, f_buf;
int buffer[BUFFER_SIZE];

void far produtor(){
    int i;
    for(i=0;i<TESTE_SIZE;i++){
        P(empty);
        P(mutex);
        buffer[f_buf] = rand() % MAX_RAND;
        fprintf(arq,"Produtor produziu: %d\n", buffer[f_buf]);
        f_buf=(f_buf+1)%BUFFER_SIZE;
        V(mutex);
        V(full);
    }
    fprintf(arq,"Produtor TERMINOU após %d iterações\n", i);
    running_bcp->status = finished;
}

void far consumidor(){
    int i;
    for(i=0;i<TESTE_SIZE;i++){
        P(full);
        P(mutex);
        fprintf(arq,"Consumidor consumiu: %d\n", buffer[i_buf]);
        printf("Consumidor consumiu: %d\n", buffer[i_buf]);
        i_buf=(i_buf+1)%BUFFER_SIZE;
        V(mutex);
        V(empty);
    }
    fprintf(arq,"Consumidor TERMINOU após %d iterações\n", i);
    running_bcp->status = finished;
}

int main(){

    process_list = (PROCESS_LIST *)malloc(sizeof(PROCESS_LIST));
    initilize_process_list(process_list);      /* Inicializa a lista de processos do escalonador*/

    arq = fopen("saida.txt", "w");
    log_arq = fopen("log.txt", "w");

    dMain = cria_desc();
    dEsc = cria_desc();
    newprocess(escalonator,dEsc);

    i_buf=0,f_buf=0;

    mutex = (SEMAPHORE *)malloc(sizeof(SEMAPHORE));
    empty = (SEMAPHORE *)malloc(sizeof(SEMAPHORE));
    full = (SEMAPHORE *)malloc(sizeof(SEMAPHORE));

    initialize_semaphore(mutex,1);
    initialize_semaphore(empty,BUFFER_SIZE);
    initialize_semaphore(full, 0);

    create_process("produtor",produtor);
    create_process("consumidor",consumidor);
    transfer(dMain, dEsc);

    fclose(arq);
    fclose(log_arq);

    return 0;
}