#include "ELigacaoDados.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS11"
#define _POSIX_SOURCE 1
#define FALSE 0
#define TRUE 1

/*flags para o alarme*/
int flag_alarme=0, conta_alarme=0;

/*flags para controlar o que é lido*/
volatile int STOP=FALSE;
int res;
unsigned char buf[256];

/*flags para controlar os estados em tramas supervisão*/
int stateI, stateF, stateM;

/*bits de controlo em envio*/
int Ns=0;

void handdler(){
  printf("alarme #%d\n", conta_alarme+1);
  flag_alarme = 1;
  conta_alarme++;
}

int llopen(int fd){
  (void) signal(SIGALRM, handdler);

  struct termios oldtio,newtio;
  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;
  newtio.c_lflag = 0;
  newtio.c_cc[VTIME] = 1;
  newtio.c_cc[VMIN] = 0;

  tcflush(fd, TCIOFLUSH);
  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }
  printf("New termios structure set\n");

  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

    unsigned char set[5];
    criarTramaSupervisor(set, SET);

    conta_alarme=0;

    while (conta_alarme<3) {
      res=write(fd,set,5);

      flag_alarme=0;
      alarm(3);

      if(verificarTramaS(fd,UA)==1) return 1;
    }

    return -1;
}

void criarTramaSupervisor(unsigned char* trama, unsigned char comando){
  trama[0]=F;
  trama[1]=A;
  trama[2]=comando;
  trama[3]=(A^comando);
  trama[4]=F;
}

unsigned char* lerTrama(int fd){
  unsigned char* rec = (unsigned char*)malloc(5);
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
    else if(stateF || flag_alarme){
      STOP=TRUE;
    }
  }
  return rec;
}

int verificarTramaS(int fd, unsigned char comando){
  unsigned char* rec=lerTrama(fd);

  alarm(0);
  if(rec[3]==(rec[1]^rec[2]) && rec[2]==comando) return 1;
  else return -1;
}

int llclose(int fd){
  unsigned char disc[5];
  criarTramaSupervisor(disc, DISC);
  unsigned char ua[5];
  criarTramaSupervisor(ua,UA);

  conta_alarme=0;

  while (conta_alarme<3) {
    res=write(fd,disc,5);

    flag_alarme=0;
    alarm(3);

    if(verificarTramaS(fd,DISC)==1) {
      write(fd,ua,5);
      return 1;
    }
  }

  return -1;
}

unsigned char calculoBCC2(unsigned char* pacote, int sizeP){
  unsigned char r = pacote[0];
  for(int i=1; i<sizeP; i++){
    r=r^pacote[i];
  }
  return r;
}

unsigned char* criarTramaI(unsigned char* pacote, int sizeP, int *sizeI){
  unsigned char* trama;
  int i=0;
  trama=(unsigned char*)malloc(sizeP+6);
  trama[i++]=F;
  trama[i++]=A;

  switch (Ns) {
    case 0:
      trama[i++]=C0;
      break;
    case 1:
      trama[i++]=C1;
      break;
    default:
      printf("Erro no bit\n");
      exit(1);
      break;
  }

  trama[i++] = (A^trama[2]);
  memcpy(trama+i, pacote, sizeP);
  i = i+sizeP;
  trama[i++] = calculoBCC2(pacote, sizeP);
  trama[i++] = F;

  *sizeI = i;

  return trama;
}

int llwrite(int fd, unsigned char* pacote, int sizePacote){
  int length;
  unsigned char* trama = criarTramaI(pacote, sizePacote, &length);

  /*printf("TramaI antes stuffing()\n");
  for(int i=0; i<length; i++){
    printf("%u ", trama[i]);
  }printf("\n");*/

  conta_alarme=0;
  int rejeitar=0,sizeTramaI;

  unsigned char* tramaI = stuffing(trama, length, &sizeTramaI);

  /*printf("TramaI depois stuffing\n");
  for(int i=0; i<sizeTramaI; i++){
    printf("%u ", tramaI[i]);
  }printf("\n");*/

  while(conta_alarme<3 || rejeitar){
    //printf("%d -> alarme %d -> rejeitar\n", conta_alarme, rejeitar);
    int res1=write(fd,tramaI,sizeTramaI);
    flag_alarme=0;
    alarm(3);


    switch (Ns) {
      case 0:
        if(verificarTramaS(fd,RR1)==1){
          Ns=1;
          return res1;
        }
        else if(verificarTramaS(fd,REJ0)){
          rejeitar=1;
        }
        break;
      case 1:
        if(verificarTramaS(fd,RR0)==1){
          Ns=0;
          return res1;
        }
        else if(verificarTramaS(fd,REJ1)){
          rejeitar=1;
        }
        break;
      default:
        printf("Erro\n");
        exit(1);
        break;
    }
  }
  return -1;
}

unsigned char* stuffing(unsigned char* trama, int length, int *sizeTramaI){
  unsigned char* tramaStuff;
  tramaStuff = (unsigned char*) malloc(2*(length+1)+5);
  int i=0;

  tramaStuff[i++]=F;
  tramaStuff[i++]=A;

  switch (Ns) {
    case 0:
      tramaStuff[i++]=C0;
      break;
    case 1:
      tramaStuff[i++]=C1;
      break;
    default:
      printf("Erro no bit\n");
      exit(1);
      break;
  }
  tramaStuff[i++] = (A^tramaStuff[2]);

  for(int j=0; j<length; j++){
    if(trama[j]==SETEE){
      tramaStuff[i++]=SETED;
      tramaStuff[i++]=CINCOE;
    }
    else if(trama[j]==SETED){
      tramaStuff[i++]=SETED;
      tramaStuff[i++]=CINCOD;
    }
    else{
      tramaStuff[i++]=trama[j];
    }
  }

  tramaStuff[i++] = F;
  *sizeTramaI = i;
  return tramaStuff;
}
