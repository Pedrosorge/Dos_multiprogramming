#ifndef NUCLEO_H
#define NUCLEO_H

#include <stdio.h>
#include <dos.h>
#include <setjmp.h>
#include "SYSTEM.H" /* Suporte para multiprogramação */
#include <stdlib.h>

/* Estados dos processos */
typedef enum {ready, finished, blocked} STATUS;

typedef struct BCP* PTR_DESC_PROC; /* Definição de ponteiro para um BCP */

typedef struct BCP{
        char name[150];
        STATUS status;
        PTR_DESC context;
        struct BCP *next;
        struct BCP *next_blocked;
} BCP; /* Definição de BCP */

typedef struct {
        PTR_DESC_PROC head;
        PTR_DESC_PROC tail;
        PTR_DESC_PROC prim;
        int num_process;
} PROCESS_LIST; /* Definição de lista de BCPs */

/* Estrutura da lista de processos bloqueados (Q)*/
typedef struct {
        PTR_DESC_PROC head;
        PTR_DESC_PROC tail;
} BLOCKED_PROCESS_QUEUE;

typedef struct{
        BLOCKED_PROCESS_QUEUE *Q;
        int count;
} SEMAPHORE; /* Definição do semáforo */

/* Estruturas necessárias para verificar se está na R.C. */
typedef struct registros{
        unsigned bx1,es1;
} regis;

typedef union k{
        regis x;
        char far *y;
} APONTA_REG_CRIT;

extern PTR_DESC_PROC running_bcp; /* Indica o processo que está sendo executado no momento */

void far volta_dos(); /* Retorna o controle para o DOS */

void far initilize_process_list(PROCESS_LIST *pl); /* Inicializa a lista de processos do escalador */
void far add_to_process_list(PROCESS_LIST *pl, BCP *bcp); /* Insere um processo na lista do escalador */

/* Equivalente a procura_prox_ativo: atualiza a cabeça da lista de processos */
int far next_bcp(PROCESS_LIST *list); 

void far add_blocked_process_to_queue(BLOCKED_PROCESS_QUEUE *bpq, PTR_DESC_PROC bcp); /* Adiciona processo a uma lista de semáforo */
void far initilize_blocked_process_queue(BLOCKED_PROCESS_QUEUE *bpq); /* Inicializa uma lista de semáforo (Q)*/
void far remove_bloqued_process_from_queue(BLOCKED_PROCESS_QUEUE *bpq); /* Remove da lista de semáforo */

void far initialize_semaphore(SEMAPHORE *s, int count); /* Inicializa um semáforo */
void far P(SEMAPHORE *s); /* Primitiva P */
void far V(SEMAPHORE *s); /* Primitiva V */

/* Equivalente a cria_processo: associa um descritor ao processo e o coloca na fila do escalador */
void far create_process(char name[150], void far (*end_proc)());

void far escalonator(); /* Co-rotina do escalador */
void far terminate_process(); /* Marca o processo como terminado */

/* Equivalente a dispara_sistema: transfere o controle para o escalonador */
void far start_system();

#endif