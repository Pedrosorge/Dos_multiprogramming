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
/* Retorna 1 caso tenha processo pronto */
/* Retorna 0 caso não tenham processos prontos */
int far next_bcp(PROCESS_LIST *list){
    int i;
    
    if(list->prim==NULL){
        return 0;
    }

    for (i = 0; i < list->num_process; i++) {
        list->prim = list->prim->next;

        if (list->prim->status != blocked && list->prim->status != finished) {
            return 1; /* Encontrou um processo pronto */
        }
    }
    
    return 0; /* Não encontrou processo pronto */
}

/* Função para inicializar fila de processos bloqueados */
void far initilize_blocked_process_queue(BLOCKED_PROCESS_QUEUE *bpq){
    bpq->head = NULL;
    bpq->tail = NULL;
}

/* Função para adicionar processo à fila de bloqueados */
void far add_blocked_process_to_queue(BLOCKED_PROCESS_QUEUE *bpq, PTR_DESC_PROC bcp){
    bcp->status = blocked; /* Marca processo como bloqueado */
    bcp->next_blocked = NULL; /* Marca próximo do processo adicionado como nulo */

    if(bpq->head == NULL){ /* Inserção na fila vazia */
        bpq->head = bcp;
        bpq->tail = bcp;
    } else { /* Insersão na fila com item */
        bpq->tail->next_blocked = bcp;
        bpq->tail = bcp;
    }
}

/* Função para remover processo da fila de bloqueados */
void far remove_bloqued_process_from_queue(BLOCKED_PROCESS_QUEUE *bpq){
    PTR_DESC_PROC temp;    
    if(bpq->head != NULL){ /* Remove o primeiro bcp da fila de bloqueados, caso tenha */
        temp = bpq->head;
        temp->status = ready;
        bpq->head = bpq->head->next_blocked;   
        if(bpq->head == NULL) bpq->tail = NULL;
    }
}

/* Função para criar novo processo */
void far create_process(char name[150], void far (*end_proc)()){
    BCP *aux = (BCP *) malloc(sizeof(BCP));                     /* Aloca BCP */
    if(aux == NULL){
        printf("teminando aux na alocação do BCP\n");
        volta_dos();
    }

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

        /* Se há processo executando rotina que não pode ser interrompida, a gente mantém a execução dele */
        if (*(a.y) != 0 && running_bcp != NULL) {
            p_est->p_destino = process_list->prim->context;
            iotransfer();
            disable();
            continue;
        }

        /* Caso não tenha, a gente procura o próximo processe pronto para por em execução */
        if (!next_bcp(process_list)) {
            volta_dos();    
        }

        if(process_list->prim != NULL){
            running_bcp = process_list->prim;
            p_est->p_destino = process_list->prim->context;
            enable();
            iotransfer();  
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

    /* Se não conseguir alocar a fila do semáforo, retorna ao dos */
    if(s->Q == NULL){
        printf("Não conseguiu alocar a fila s->Q\n");
        volta_dos();
    }

    initilize_blocked_process_queue(s->Q);
}

/* Diretiva P dos semaforos */
void far P(SEMAPHORE *s){
    PTR_DESC aux_context; 
    
    disable(); /* Desabilita interrupções */
    
    if(s->count > 0){ /* Verifica se o contador do semáforo pode ser decrementado*/
        s->count--;
        enable();
    }
    else{ /* Se não puder ser decrementado, bloqueia processo */
        add_blocked_process_to_queue(s->Q, process_list->prim); /* Isere o proceso atual na lista de blockeados */
        enable();
        transfer(process_list->prim->context, dEsc); /* Transfere o controle da UCP para o ecalonador */
    }
}


/* Diretiva V dos semáforos*/
void far V(SEMAPHORE *s){
    disable();
    if(s->Q->head != NULL){ /* Verifica se a fila do semáforo tem algum processo */
        remove_bloqued_process_from_queue(s->Q); /* Remove o processo da fila de bloqueados */
    }
    else{ /* Se não tiverem processos bloqueados, incrementa do contador */
        s->count++; 
    }
    enable();
}

