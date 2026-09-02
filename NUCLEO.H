#ifndef NUCLEO_H
#define NUCLEO_H
#include <stdio.h>
#include <dos.h>
#include <setjmp.h>
#include <stdlib.h>

typedef enum {ready, finished, blocked} STATUS;

typedef struct BCP* PTR_DESC_PROC; /* Definição de ponteiro para um BCP */

typedef struct BCP{
        char name[150];
        STATUS status;
        PTR_DESC context;
        struct BCP *next;
} BCP; /* Definição de BCP */
typedef struct {
        PTR_DESC_PROC head;
        PTR_DESC_PROC tail;
        PTR_DESC_PROC prim;
        int num_process;
} PROCESS_LIST; /* Definição de lista de BCPs */

typedef struct BLOCKED_QUEUE_NODE{
        PTR_DESC_PROC bcp;
        struct BLOCKED_QUEUE_NODE *next;
} BLOCKED_QUEUE_NODE; /* Definição do nó da fila de bloqueados*/

typedef struct{
        BLOCKED_QUEUE_NODE *head;
        BLOCKED_QUEUE_NODE *tail;
} BLOCKED_PROCESS_QUEUE; /* Definição da fila de bloqueados */

typedef struct{
        BLOCKED_PROCESS_QUEUE *Q;
        int count;
} Semaphore; /* Definição de Semaphore */
void initilize_process_list(PROCESS_LIST *pl);
void add_to_process_list(PROCESS_LIST *pl, BCP *bcp);
int next_bcp(PROCESS_LIST *list);
void initilize_blocked_process_queue(BLOCKED_PROCESS_QUEUE *bpq);
void add_blocked_process_to_queue(BLOCKED_PROCESS_QUEUE *bpq, PTR_DESC_PROC bcp);
void remove_bloqued_process_from_queue(BLOCKED_PROCESS_QUEUE *bpq);
void far create_process(char name[150], void far (*end_proc)());
void far escalonator();
#endif