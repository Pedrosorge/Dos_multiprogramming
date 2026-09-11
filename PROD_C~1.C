#include "NUCLEO.H"
#include "SYSTEM.H"
#include <stdio.h>

#define TESTE_SIZE 200
#define BUFFER_SIZE 5
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
        fprintf(arq,"%s produziu: %d\n", running_bcp->name,buffer[f_buf]);
        fflush(arq);
        f_buf=(f_buf+1)%BUFFER_SIZE;
        V(mutex);
        V(full);
    }
    fprintf(arq,"%s TERMINOU após %d iterações\n", running_bcp->name, i);
    fflush(arq);
    running_bcp->status = finished;
    while(1); 
}

void far consumidor(){
    int i;
    for(i=0;i<TESTE_SIZE;i++){
        P(full);
        P(mutex);
        fprintf(arq,"%s consumiu: %d\n", running_bcp->name,buffer[i_buf]);
        fflush(arq);
        i_buf=(i_buf+1)%BUFFER_SIZE;
        V(mutex);
        V(empty);
    }
    fprintf(arq,"%s TERMINOU após %d iterações\n", running_bcp->name,i);
    fflush(arq);
    running_bcp->status = finished;
    while(1); 
}

int main(){

    process_list = (PROCESS_LIST *)malloc(sizeof(PROCESS_LIST));
    
    if(process_list == NULL){
        fprintf(log_arq,"Não foi possível alocar process_list!!");
        fflush(log_arq);
    }

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

    create_process("produtor1",produtor);
    create_process("produtor2",produtor);
    create_process("consumidor1",consumidor);
    create_process("consumidor2",consumidor);
    transfer(dMain, dEsc);

    fclose(arq);
    fclose(log_arq);

    return 0;
}