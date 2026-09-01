#include "STDIO.H"
#include "SYSTEM.H"

PTR_DESC dMain;
PTR_DESC dTic;
PTR_DESC dTac;

void far tic(){
     while(1){
          printf("tic ");
          transfer(dTic, dTac);
     }
}

void far tac(){
     while(1){
          printf("tac\n");
          transfer(dTac, dTic);
     }
}

int main(){

     dMain = cria_desc();
     dTic = cria_desc();
     dTac = cria_desc();

     newprocess(tic,dTic);
     newprocess(tac,dTac);
     
     transfer(dMain,dTic);

    return 0;
}