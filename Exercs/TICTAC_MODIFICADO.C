#include "STDIO.H"
#include "SYSTEM.H" /* Suporte para multiprogramação */

/* Descritores das co-rotinas */
PTR_DESC dMain;
PTR_DESC dTic;
PTR_DESC dTac;

/* Constante que determina o número de repetições */
int COUNT = 100;

/* Co-rotina que imprime tic */
void far tic(){

     /* Repete o laço até que COUNT seja 0 */
     while(COUNT) {
          printf("tic-");
          transfer(dTic, dTac); /* Transfere o controle para tac */
     }

     transfer(dTic, dMain); /* Retorna o controle para a main */

}

/* Co-rotina que imprime tac */
void far tac(){

     while(1) {
          printf("tac\n");
          COUNT--; /* Decrementa COUNT */
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
     
     /* Inicializa o laço */
     transfer(dMain,dTic);

    return 0;

}