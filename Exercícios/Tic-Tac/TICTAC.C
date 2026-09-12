#include "../../Dependências/STDIO.H"
#include "../../Dependências/SYSTEM.H" /* Suporte para multiprogramação */

/* Descritores das co-rotinas */
PTR_DESC dMain; 
PTR_DESC dTic;
PTR_DESC dTac;

/* Co-rotina que imprime tic */
void far tic(){
     while(1){
          printf("tic ");
          transfer(dTic, dTac); /* Transfere o controle para tic */
     }
}

/* Co-rotina que imprime tac */
void far tac(){
     while(1){
          printf("tac\n");
          transfer(dTac, dTic); /* Transfere o controle para tac */
     }
}

int main(){

     /* Inicializa os descritores */
     dMain = cria_desc();
     dTic = cria_desc();
     dTac = cria_desc();

     /* Atribui cada descritor a uma co-rotina */
     newprocess(tic,dTic);
     newprocess(tac,dTac);
     
     /* Inicializa o loop */
     transfer(dMain,dTic);

    return 0;
}