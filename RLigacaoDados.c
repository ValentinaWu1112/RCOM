#include "RLigacaoDados.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define FALSE 0
#define TRUE 1

#define MAXSIZE 1024

/*flags para controlar o que é lido*/
volatile int STOP=FALSE;
int res;
unsigned char buf[256];

/*flags para controlar os estados em tramas supervisão*/
int stateI, stateF, stateM;

/*bits de controlo em envio*/
int Nr=0;

/*tamanho da trama lida*/
int size;

int llopen(int fd){

  unsigned char ua[5];
  criarTramaSupervisor(ua,UA);

  if(verificarTramaS(fd,SET)==1){
    fflush(stdout);
    res=write(fd,ua,5);
    return 1;
  }
  return -1;
}

int verificarTramaS(int fd, unsigned char comando){
  unsigned char *rec;
  rec = lerTrama(fd);

  if(rec[3]==(rec[1]^rec[2]) && rec[2]==comando) return 1;
  else return -1;
}

void criarTramaSupervisor(unsigned char* trama, unsigned char comando){
  trama[0]=F;
  trama[1]=A;
  trama[2]=comando;
  trama[3]=(A^comando);
  trama[4]=F;
}

int llclose(int fd){
  unsigned char disc[5];
  criarTramaSupervisor(disc,DISC);

  if(verificarTramaS(fd,DISC)==1){
    fflush(stdout);
    res=write(fd,disc,5);
  }

  if(verificarTramaS(fd,UA)==1){
    return 1;
  }

  return -1;
}

unsigned char* lerTrama(int fd){
  unsigned char *rec;
  rec=(unsigned char*)malloc(MAXSIZE);
  int i=0;
  STOP=FALSE;
  stateI=1, stateF=0, stateM=0;

  while (STOP==FALSE) {
    res=read(fd,buf,1);
    if(res>0){
      if(buf[0]==F && stateI){ //inicio da leitura
        rec[i]=buf[0];
        stateI=0;
        stateM=1;
        i++;
      }
      else if(buf[0]==F && stateM){ //fim da leitura
        rec[i]=buf[0];
        stateM=0;
        stateF=1;
        i++;
      }
      else if(stateM){ //estado no meio
        rec[i]=buf[0];
        i++;
      }
    }
    else if(stateF){
      STOP=TRUE;
    }
  }
  size=i;
  fflush(stdout);
  return rec;
}


int llread(int fd, unsigned char* buffer){
  unsigned char* tmp;
  unsigned char c;
  tmp = lerTrama(fd);
  c=tmp[2];

  int sizeTramaI, sizePacote;
  unsigned char* tmp1;
  unsigned char* pacote;
  unsigned char bcc;
  if(tmp[3]==(tmp[1]^tmp[2])) {
    tmp1 = destuffing(tmp, &sizeTramaI);
    bcc = tmp1[sizeTramaI-2];
    pacote=tirarHeaders(tmp1,sizeTramaI, &sizePacote);
  }

  unsigned char* controlo;

  if(verificarBCC2(pacote, sizePacote, bcc)){
    /*Mandar a mensagem para buffer*/
    memcpy(buffer,pacote, sizePacote);
    switch (Nr) {
      case 0:
        if(c==C0){
          Nr=1;
          controlo = (unsigned char*) malloc(5);
          criarTramaSupervisor(controlo, RR1);
          fflush(stdout);
          write(fd, controlo, 5);
        }
        else if(c==C1){
          controlo = (unsigned char*) malloc(5);
          criarTramaSupervisor(controlo, RR0);
          fflush(stdout);
          write(fd, controlo, 5);
        }
        break;
      case 1:
        if(c==C1){
          Nr=0;
          controlo = (unsigned char*) malloc(5);
          criarTramaSupervisor(controlo, RR0);
          fflush(stdout);
          write(fd, controlo, 5);
        }
        else if(c==C0){
          controlo = (unsigned char*) malloc(5);
          criarTramaSupervisor(controlo, RR1);
          fflush(stdout);
          write(fd, controlo, 5);
        }
        break;
    }
  }
  else{
    switch (Nr) {
      case 0:
        controlo = (unsigned char*) malloc(5);
        criarTramaSupervisor(controlo, REJ0);
        fflush(stdout);
        write(fd, controlo, 5);
        break;
      case 1:
        controlo = (unsigned char*) malloc(5);
        criarTramaSupervisor(controlo, REJ1);
        fflush(stdout);
        write(fd, controlo, 5);
        break;
    }
  }

  return sizePacote;
}

unsigned char* tirarHeaders(unsigned char* tramaI, int sizeTramaI, int *sizePacote){
  unsigned char* pacote;
  pacote = (unsigned char*) malloc (sizeTramaI-6);

  int j=0;
  for(int i=4; i<sizeTramaI-2; i++){
    pacote[j++]=tramaI[i];
  }

  *sizePacote = j;
  return pacote;
}

int verificarBCC2(unsigned char* pacote, int sizePacote, unsigned char bcc){
  /*cacular BCC2*/
  unsigned char bcc2 = pacote[0];
  for(int i=1; i<sizePacote; i++)
    bcc2 = bcc2 ^ pacote[i];

  return (bcc2==bcc);
}

unsigned char* destuffing(unsigned char* tramaI, int *sizeTrama){
  unsigned char* r;
  r=(unsigned char*)malloc(MAXSIZE);
  /*i=4 porque as tramas de informação vieram com headers
  acaba em size-1 por causa da flag final*/
  int i=4, j=0;
  while (i<size-1) {
    if(tramaI[i]==SETED && tramaI[i+1]==CINCOE){
      r[j]=SETEE;
      i+=2;
      j++;
    }
    else if(tramaI[i]==SETED && tramaI[i+1]==CINCOD){
      r[j]=SETED;
      i+=2;
      j++;
    }
    else{
      r[j]=tramaI[i];
      j++;
      i++;
    }
  }
  *sizeTrama = j;
  return r;
}
