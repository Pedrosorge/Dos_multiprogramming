#include "SYSTEM.H" /* Suporte para multiprogramação */
#include <dos.h>
#include <string.h>
#include "NUCLEO.H" /* Cabeçalho */

PROCESS_LIST *process_list; /* Lista de processos do escalonador*/

/* Descritores da main e do escalador */
PTR_DESC dMain;
PTR_DESC dEsc;

extern FILE *log_arq;

APONTA_REG_CRIT a;

PTR_DESC_PROC running_bcp = NULL; /* Indica o processo que está sendo executado */

/* Realoca a interrupção anterior que estava configurada no DOS */
void far volta_dos(){

    disable();

    setvect(p_est->num_vetor,p_est->int_anterior);
    p_est->p_destino = dMain;

    enable();

    transfer(dEsc,dMain); /* Retorna para main */

}

/* Função para inicializar lista de BCPs*/
void far initilize_process_list(PROCESS_LIST *pl){
    pl->head = NULL;
    pl->tail = NULL;
    pl->prim = NULL;
    pl->num_process=0;
}

/* Função para adicionar BCP do processo na lista do escalonador */
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
        temp->status = ready; /* Muda o estado do processo removido para 'ativo' */
        bpq->head = bpq->head->next_blocked;   
        if(bpq->head == NULL) bpq->tail = NULL;
    }
}

/* Função para criar novo processo */
void far create_process(char name[150], void far (*end_proc)()){

    BCP *aux = (BCP *) malloc(sizeof(BCP)); /* Aloca um BCP */
    if(aux == NULL){
        printf("Falha na alocação do BCP\n");
        exit(1);
    }

    strcpy(aux->name, name); /* Compia o nome do processo */
    aux->context = cria_desc(); /* Cria descritor para contexto do processo*/
    newprocess(end_proc, aux->context); /* Inicializa o descritor de contexto */
    aux->status = ready; /* Inicializa o estado do processo */
    
    add_to_process_list(process_list, aux); /* Adiciona o BCP na lista de processos do escalonador */
}


/* Função da corrotina do escalonador de processos */
void far escalonator(){
    
    p_est->num_vetor = 8; /* Indica a posição da interrupção que deve ser alterada na tebela de interrupções */
    p_est->p_origem = dEsc; /* Indica que a origem da rotina de interrupção é o escalonador */
    p_est->p_destino = process_list->prim->context; /* Indica que o processo para o qual o controle será transferido é o primeiro na fila do escalador */
    running_bcp = process_list->prim; /* Indica o processo que será executado */

    /* Configura os registradores para saber se alguma rotina que não pode ser interrompida está sendo executada */
    _AH=0x34;                   
    _AL=0x00;
    geninterrupt(0x21);
    a.x.bx1=_BX;
    a.x.es1=_ES;

    while(1){

        iotransfer(); /* Escalona o primeiro processo ativo na fila */
        disable();

        /* Se não há processo executando rotina que não pode ser interrompida, busca o próximo ativo */
        if (*(a.y) == 0) {

            /* Busca próximo processo da lista ativo */
            if (!next_bcp(process_list)) {
                volta_dos(); /* Retorna o controle para o DOS caso não haja nenhum processo ativo */
            }

            running_bcp = process_list->prim; /* Atualiza o processo que será executado */
            p_est->p_destino = process_list->prim->context; /* Altera o destino do iotransfer */

        }

        enable();
    } 
}

/* Inicializa um semáforo*/
void far initialize_semaphore(SEMAPHORE *s, int count){

    s->count=count; /* Inicializa o contador */

    /* Aloca e inicializa a fila do semáforo*/
    s->Q = (BLOCKED_PROCESS_QUEUE *) malloc(sizeof(BLOCKED_PROCESS_QUEUE));
    if(s->Q == NULL){
        printf("Erro na alocação da fila do semáforo\n");
        exit(1);
    }
    initilize_blocked_process_queue(s->Q);

}

/* Diretiva P dos semaforos */
void far P(SEMAPHORE *s){
    
    disable(); /* Desabilita interrupções */
    
    if(s->count > 0){ /* Verifica se o contador do semáforo pode ser decrementado */
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

/* Transfere o controle da main para o escalador */
void far start_system() {

    /* Inicializa os descritores da main e do escalador */
    dEsc = cria_desc();
    dMain = cria_desc();

    newprocess(escalonator, dEsc); /* Associa o descritor a rotina do escalador */
    transfer(dMain, dEsc); /* Transfere o controle da main para o escalador */

}

/* Marca o processo chamador como 'finished' */
void far terminate_process() {

    disable();
    running_bcp->status = finished;
    enable();
    while(1); /* Laço eterno para que o controle retorne para o escalador após interrupção */

}