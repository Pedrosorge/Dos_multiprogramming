#include "NUCLEO.H" /* Núclo multiprogramado */
#include <stdio.h>

#define TESTE_SIZE 200 /* Número de iterações de cada processo */
#define BUFFER_SIZE 5 /* Tamanho do buffer */

#define MAX_RAND 20

/* Declara a lista de processos e os descritores da main e do escalador no escopo deste arquivo */
extern PTR_DESC dMain, dEsc;
extern PTR_DESC dMain, dEsc;
extern PROCESS_LIST *process_list;

/* Declara os semáforos para acesso e uso do buffer */
SEMAPHORE *mutex, *empty, *full;

FILE *arq; /* Arquivo com saídas dos testes */
FILE *log_arq; 

int i_buf, f_buf;
int buffer[BUFFER_SIZE]; /* Buffer */

/* Processo produtor */
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
    
    terminate_process();
}

/* Processo consumidor */
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
    
    terminate_process();
}

int main(){

    arq = fopen("saida.txt", "w");
    log_arq = fopen("log.txt", "w");

    /* Inicializa a lista de processos do escalonador */
    process_list = (PROCESS_LIST *)malloc(sizeof(PROCESS_LIST));

    if(process_list == NULL){
        fprintf(log_arq,"Não foi possível alocar process_list");
        fflush(log_arq);
        return 1;
    }

    initilize_process_list(process_list);      

    i_buf=0,f_buf=0;


    mutex = (SEMAPHORE *)malloc(sizeof(SEMAPHORE));
    empty = (SEMAPHORE *)malloc(sizeof(SEMAPHORE));
    full = (SEMAPHORE *)malloc(sizeof(SEMAPHORE));

    /* Inicializa os semáforos */
    initialize_semaphore(mutex,1);
    initialize_semaphore(empty,BUFFER_SIZE);
    initialize_semaphore(full, 0);

    /* Cria os processos que serão escalonados */
    create_process("produtor1",produtor);
    create_process("produtor2",produtor);
    create_process("consumidor1",consumidor);
    create_process("consumidor2",consumidor);
    
    /* Dispara o sistema */
    start_system();

    fclose(arq);
    fclose(log_arq);

    return 0;
}