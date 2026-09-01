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
    if(list->prim==NULL) return 0;
    int aux=0;
    while(list->prim->STATUS == blocked){
        list->prim = list->prim->next;
        if(++aux == list->num_process){
            exit(0);
            volta_dos(); /* FALTA IMPLEMENTAR*/
        }
    }
    return 1;
}


/* Função para criar novo processo */
void far create_process(char name[150], void far (*end_proc)()){
    BCP *aux = (BCP *) malloc(sizeof(BCP));                     /* Aloca BCP */
    strcpy(aux->name, name);                                    /* Compia o nome do processo */
    aux->context = (PTR_DESC_PROC) malloc(sizeof(PTR_DESC));    /* Aloca descritor do processo*/
    *(aux->context) = cria_desc();                              /* Cria descritor do processo */
    newprocess(end_proc, *(aux->context));                      /* Inicializa o descritor */
    aux->status = ready;                                        /* Inicializa o estado do processo */
    
    add_to_process_list(&process_list, aux);                    /* Adiciona o bcp na lista de processos do escalonador */
}

void far escalonator(){
    p_est->num_vetor = 8;
    p_est->origem = dEsc;
    
    while(1){
        if(process_list.prim != NULL){
            p_est->p_destino = *(process_list.prim->context);
            iotransfer(); 
        }
        if (!next_bcp(&process_list)) exit(0);
    }

}


int main(){
    initilize_process_list(&process_list); /* Inicializa a lista de processos */

    dMain = cria_desc();
    dEsc = cria_desc();
    newprocess(escalonator,dEsc);

    transfer(dMain, dEsc);

    return 0;

}