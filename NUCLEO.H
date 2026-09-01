#include <stdio.h>
#include <dos.h>
#include <setjmp.h>
#include <stdlib.h>
#include "SYSTEM.H"

typedef enum {ready, finished, blocked} STATUS;

typedef BCP* PTR_DESC_PROC;

typedef struct {
    PTR_DESC_PROC head;
    PTR_DESC_PROC tail;
    PTR_DESC_PROC prim;
    int num_process;
} PROCESS_LIST;

typedef struct BCP{
    char name[150];
    STATUS status;
    PTR_DESC_PROC context;
    struct BCP *next;
} BCP;


void initilize_process_list(PROCESS_LIST *pl);
void add_to_process_list(PROCESS_LIST *pl, BCP *bcp);

int next_bcp(PROCESS_LIST *list);

void far create_process(char name[150], void far (*end_proc)());

void far escalonator();



