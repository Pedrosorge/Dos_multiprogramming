#include "SYSTEM.H"
#include <dos.h>
#include <string.h>
#include "NUCLEO.H"

PROCESS_LIST *process_list; /* Lista de processos do escalonador*/
PTR_DESC dMain;
PTR_DESC dEsc;
extern FILE *log_arq;

APONTA_REG_CRIT a;
PTR_DESC_PROC running_bcp = NULL;

/* Realoca a interrupção anterior que estava configurada no dos */
void far volta_dos(){

    disable();

    setvect(p_est->num_vetor,p_est->int_anterior);
    p_est->p_destino = dMain;

    enable();

    transfer(dEsc,dMain);
    exit(0);

}

/* Função para inicializar lista de bcps*/
void far initilize_process_list(PROCESS_LIST *pl){
    pl->head = NULL;
    pl->tail = NULL;
    pl->prim = NULL;
    pl->num_process=0;
}

/* Função para adicionar bcp to processo na lista do escalonador */
void far add_to_process_list(PROCESS_LIST *pl, BCP *bcp){
    if(pl->head == NULL){
        pl->head = bcp;
        pl->tail = bcp;
        pl->prim = bcp;
        bcp->next = bcp;
    }
    else{
        pl->tail->next = bcp;
        pl->tail = bcp;
        bcp->next = pl->head;
    }
    pl->num_process++;
}

/* Função para achar o próximo processo disponível */
int far next_bcp(PROCESS_LIST *list){
    int i;
    char s;
    if(list->prim==NULL){
        fprintf(log_arq,"Fila vazia!!\n");
        return 0;
    }

    for (i = 0; i < list->num_process; i++) {
        list->prim = list->prim->next;

        if(list->prim->status == blocked) s = 'B';
        else if(list->prim->status == finished) s = 'F';
        else s = 'A';

        fprintf(log_arq,"Nome: %s Estado: %c\n", list->prim->name, s);
        if (list->prim->status != blocked && list->prim->status != finished) {
            return 1; /* Encontrou um processo pronto */
        }
    }
    
    fprintf(log_arq,"Não encontrou processo desbloqueado!\n");
    return 0;
}

/* Função para inicializar fila de processos bloqueados */
void far initilize_blocked_process_queue(BLOCKED_PROCESS_QUEUE *bpq){
    bpq->head = NULL;
    bpq->tail = NULL;
}

/* Função para adicionar processo à fila de bloqueados */
void far add_blocked_process_to_queue(BLOCKED_PROCESS_QUEUE *bpq, PTR_DESC_PROC bcp){

    BLOCKED_QUEUE_NODE *aux = (BLOCKED_QUEUE_NODE *) malloc(sizeof(BLOCKED_QUEUE_NODE)); /* Instancia nó que aponta para o processo bloqueado */
    aux->bcp = bcp; /* Inicializa o nó */
    aux->next = NULL;
    bcp->status = blocked;

    if(bpq->head == NULL){
        bpq->head = aux;
        bpq->tail = aux;
    }
    else{
        bpq->tail->next = aux;
        bpq->tail = aux;
    }
    fprintf(log_arq, "add_blocked: bpq=%p head=%p head->bcp=%s\n", bpq, bpq->head, bpq->head->bcp->name);
}

/* Função para remover processo da fila de bloqueados */
void far remove_bloqued_process_from_queue(BLOCKED_PROCESS_QUEUE *bpq){

    BLOCKED_QUEUE_NODE *temp;    
    if(bpq->head != NULL){
        temp = bpq->head;
        temp->bcp->status = ready;
        bpq->head = bpq->head->next;   
        if(bpq->head == NULL) bpq->tail = NULL;
        free(temp);
    }

}


/* Função para criar novo processo */
void far create_process(char name[150], void far (*end_proc)()){
    BCP *aux = (BCP *) malloc(sizeof(BCP));                     /* Aloca BCP */
    strcpy(aux->name, name);                                    /* Compia o nome do processo */
    aux->context = cria_desc();                                 /* Cria descritor do processo*/
    newprocess(end_proc, aux->context);                         /* Inicializa o descritor */
    aux->status = ready;                                        /* Inicializa o estado do processo */
    
    add_to_process_list(process_list, aux);                    /* Adiciona o bcp na lista de processos do escalonador */
}


/* Função da corrotina do escalonador de processos */
void far escalonator(){
    
    p_est->num_vetor = 8;       /* Indica a posição da interrupção que deve ser alterada na tebela de interrupções */
    p_est->p_origem = dEsc;     /* Indica que a origem da rotina de interrupção é o escalonador */

    /* Configura os registradores para saber se alguma rotina que não pode ser interrompida está sendo executada */
    _AH=0x34;                   
    _AL=0x00;
    geninterrupt(0x21);
    a.x.bx1=_BX;
    a.x.es1=_ES;

    while(1){

        if (!next_bcp(process_list)) {
            volta_dos();    
        }

        if(process_list->prim != NULL){
            if(*(a.y) == 0) {
                fprintf(log_arq,"Iniciou execução de %s!\n", process_list->prim->name);
                running_bcp = process_list->prim;
                p_est->p_destino = process_list->prim->context;
                iotransfer();  
            }
        }
    } 
}


/*========================================*/
/* Funções para projetos multiprogramados */
/*========================================*/

/* Inicializa semáforos*/
void far initialize_semaphore(SEMAPHORE *s, int count){
    s->count=count;
    s->Q = (BLOCKED_PROCESS_QUEUE *) malloc(sizeof(BLOCKED_PROCESS_QUEUE));
    initilize_blocked_process_queue(s->Q);
}

/* Diretica P dos semáforos*/
void far P(SEMAPHORE *s){
    PTR_DESC aux_context; 
    
    disable();
    fprintf(log_arq, "P() semaforo=%p fila=%p count=%d\n", s, s->Q, s->count);
    if(s->count>0){
        fprintf(log_arq, "P() entrou no if, count=%d\n", s->count);
        s->count--;
        enable();
    }
    else{
        fprintf(log_arq, "P() entrou no else, bloqueando %s\n", process_list->prim->name);
        add_blocked_process_to_queue(s->Q,process_list->prim);
        aux_context = process_list->prim->context;
        enable();
        transfer(aux_context, dEsc);
    }
}

/* Diretiva V dos semáforos*/
void far V(SEMAPHORE *s){
    disable();
    fprintf(log_arq, "V() semaforo=%p fila=%p head=%s\n", s, s->Q, s->Q->head ? s->Q->head->bcp->name : "NULL");
    if(s->Q->head != NULL){
        remove_bloqued_process_from_queue(s->Q);
    }
    else{
        s->count++;
    }
    enable();
}

