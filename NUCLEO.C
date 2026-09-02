#include "SYSTEM.H"
#include "STRING.H"
#include "NUCLEO.H"

PROCESS_LIST process_list; /* Lista de processos do escalonador*/
PTR_DESC dMain;
PTR_DESC dEsc;

/* Função para inicializar lista de bcps*/
void initilize_process_list(PROCESS_LIST *pl){
    pl->head = NULL;
    pl->tail = NULL;
    pl->prim = NULL;
    pl->num_process=0;
}

/* Função para adicionar bcp to processo na lista do escalonador */
void add_to_process_list(PROCESS_LIST *pl, BCP *bcp){
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
int next_bcp(PROCESS_LIST *list){
    int i;
    if(list->prim==NULL) return 0;
    
    list->prim = list->prim->next;

    for (i = 0; i < list->num_process; i++) {
        if (list->prim->status != blocked) {
            return 1; /* Encontrou um processo pronto */
        }
        list->prim = list->prim->next;
    }

    volta_dos();
    exit(0);
    return 0;
}

/* Função para inicializar fila de processos bloqueados */
void initilize_blocked_process_queue(BLOCKED_PROCESS_QUEUE *bpq){
    bpq->head = NULL;
    bpq->tail = NULL;
}

/* Função para adicionar processo à fila de bloqueados */
void add_blocked_process_to_queue(BLOCKED_PROCESS_QUEUE *bpq, PTR_DESC_PROC bcp){

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

}

/* Função para remover processo da fila de bloqueados */
void remove_bloqued_process_from_queue(BLOCKED_PROCESS_QUEUE *bpq){

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
    
    add_to_process_list(&process_list, aux);                    /* Adiciona o bcp na lista de processos do escalonador */
}

/* Função da corrotina do escalonador de processos */
void far escalonator(){
    p_est->num_vetor = 8;
    p_est->p_origem = dEsc;
    
    while(1){
        if(process_list.prim != NULL){
            p_est->p_destino = process_list.prim->context;
            iotransfer(); 
        }
        if (!next_bcp(&process_list)) exit(0);
    }

}


/* Código principal */
int main(){
    initilize_process_list(&process_list); /* Inicializa a lista de processos */

    dMain = cria_desc();
    dEsc = cria_desc();
    newprocess(escalonator,dEsc);

    transfer(dMain, dEsc);

    return 0;

}