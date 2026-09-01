#include "STDIO.H"
#include "SYSTEM.H"

typedef struct ESCALATOR_NODE {
    PTR_DESC process;
    struct ESCALATOR_NODE *next;
} ESCALATOR_NODE;

typedef struct {
        ESCALATOR_NODE *head;
        ESCALATOR_NODE *tail;
} ESCALATOR_LIST;

void initialize_list(ESCALATOR_LIST *list){
    list->head = NULL;
    list->tail = NULL;
}


ESCALATOR_NODE* create_node(PTR_DESC desc){
    ESCALATOR_NODE *node = (ESCALATOR_NODE*) malloc(sizeof(ESCALATOR_NODE));
    node->next = NULL;
    node->process = desc;
    return node;
}

void create_process(ESCALATOR_LIST *list, PTR_DESC desc){
    ESCALATOR_NODE *temp = create_node(desc);
     if(list->head == NULL){
             list->head = create_node(desc);
             list->tail = list->head;
             list->tail->next = list->head;
             return;
     }
     temp->next = list->head;
     list->tail->next = temp;
     list->tail = temp;
}
void far tic(){
     while(1){
            printf("tic1");
     }
}
void far tac(){
     while(1){
            printf("tac2\n");
     }
}

PTR_DESC dMain;
PTR_DESC dTic;
PTR_DESC dTac;
PTR_DESC dEscalator;
ESCALATOR_LIST processos;

void far escalator(){
    ESCALATOR_NODE *curr_node = processos.head;
    p_est->num_vetor = 8;
    p_est->p_origem = dEscalator;
    do{
       p_est->p_destino = curr_node->process;
       iotransfer();
       curr_node = curr_node->next;
    } while(1);
}

int main(){
    initialize_list(&processos);
    dMain = cria_desc();
    dTic = cria_desc();
    dTac = cria_desc();
    dEscalator = cria_desc();
    newprocess(tic, dTic);
    newprocess(tac, dTac);
    newprocess(escalator, dEscalator);
    create_process(&processos, dTic);
    create_process(&processos, dTac);
    transfer(dMain, dEscalator);
    return 0;
}