#include "../../Dependências/STDIO.H"
#include "../../Dependências/SYSTEM.H" /* Suporte para multiprogramação */

typedef struct ESCALATOR_NODE {
    PTR_DESC process;
    struct ESCALATOR_NODE *next;
} ESCALATOR_NODE;

/* Lista circular de co-rotinas que serão escalonadas */
typedef struct {
        ESCALATOR_NODE *head;
        ESCALATOR_NODE *tail;
} ESCALATOR_LIST;

/* Inicializa a lista do escalonador */
void initialize_list(ESCALATOR_LIST *list){
    list->head = NULL;
    list->tail = NULL;
}

/* Cria um nó para que a co-rotina possa ser adicionada a lista do escalonador */
ESCALATOR_NODE* create_node(PTR_DESC desc){
    ESCALATOR_NODE *node = (ESCALATOR_NODE*) malloc(sizeof(ESCALATOR_NODE));
    node->next = NULL;
    node->process = desc;
    return node;
}

/* Insere a co-rotina na lista do escalonador */
void create_process(ESCALATOR_LIST *list, PTR_DESC desc){
    ESCALATOR_NODE *temp = create_node(desc);
     if(list->head == NULL){
             list->head = temp;
             list->tail = list->head;
             list->tail->next = list->head;
             return;
     }
     temp->next = list->head;
     list->tail->next = temp;
     list->tail = temp;
}

/* Co-rotinas */
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

/* Descritores das co-rotinas */
PTR_DESC dMain;
PTR_DESC dTic;
PTR_DESC dTac;
PTR_DESC dEscalator;

/* Lista de co-rotinas do escalonador */
ESCALATOR_LIST processos;

/* Escalonador */
void far escalator(){
    ESCALATOR_NODE *curr_node = processos.head; /* Cabeça da lista (prim) */

    /* Parâmetros de iotransfer */
    p_est->num_vetor = 8; 
    p_est->p_origem = dEscalator;
    /* Escalona ambas as co-rotinas alternadamente */
    do{
       p_est->p_destino = curr_node->process;
       iotransfer();
       curr_node = curr_node->next;
    } while(1);

}

int main(){
    /* Inicializa a lista do escalonador */
    initialize_list(&processos);

    /* Incializa os descritores das co-rotinas */
    dMain = cria_desc();
    dTic = cria_desc();
    dTac = cria_desc();
    dEscalator = cria_desc();

    /* Atribui cada descritor a sua co-rotina */
    newprocess(tic, dTic);
    newprocess(tac, dTac);
    newprocess(escalator, dEscalator);

    /* Adiciona as co-rotinas tic-tac na lista do escalonador */
    create_process(&processos, dTic);
    create_process(&processos, dTac);

    /* Inicia o escalonamento */
    transfer(dMain, dEscalator);
    return 0;
}