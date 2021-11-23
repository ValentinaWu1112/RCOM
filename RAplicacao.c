#include "RAplicacao.h"
#include "RLigacaoDados.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1
#define FALSE 0
#define TRUE 1

#define MAXSIZE 1024

/*Mensagem recebida*/
unsigned char* mensagem;
int ind=0;
int packages=0;

/*Informação recebida do ficheiro*/
char *nomeFicheiro;
long int sizeFicheiro;

void call_llopen(int fd){
  if(llopen(fd)==1) printf("Ligação estabelecidda com sucesso\n");
  else{
    printf("Ligação falhada\n");
    exit(1);
  }
}

void call_llclose(int fd){
  if(llclose(fd)==1) printf("Terminada com sucesso\n");
  else{
    printf("Terminada sem sucesso\n");
    exit(1);
  }
}

void recetor(int fd){
  unsigned char* pacote = (unsigned char*) malloc (MAXSIZE);
  int sizePacote = llread(fd, pacote);
  /*Receção da trama Start*/
  analisarPacote(pacote, sizePacote);

  ind=0;
  while(packages>0){
    free(pacote);
    pacote = (unsigned char*) malloc (MAXSIZE);
    sizePacote = llread(fd, pacote);
    analisarPacote(pacote, sizePacote);
    packages--;
  }

  ficheiro();

  free(pacote);
  pacote = (unsigned char*) malloc (MAXSIZE);
  sizePacote = llread(fd, pacote);
  /*Receção trama END*/
  analisarPacote(pacote, sizePacote);
}

void analisarPacote(unsigned char* pacote, int sizePacote){
  unsigned char c = pacote[0];
  switch (c) {
    case START:
      pacoteStart(pacote, sizePacote);
      printf("Trama START bem recebida\n");
      break;
    case END:
      printf("Trama END bem recebida\n");
      break;
    case DADOS:
      pacoteDados(pacote,sizePacote);
      printf("Trama I bem recebida com %d bytes, pacote %d\n", sizePacote, (int)pacote[1]);
      break;
  }
}

void pacoteStart(unsigned char* pacote, int sizePacote){
  off_t tamanho = 0x00;
  int index=0;
  if(pacote[1]==T1){ //Tamaho do ficheiro
    packages = (int)pacote[2];
    for(int i=3; i<3+packages; i++){
      tamanho+=pacote[i];
    }
    sizeFicheiro = (long int) tamanho;
    index=3+packages;
  }

  int lengthName, j=0 ;
  if(pacote[index++]==T2){//Nome do ficheiro;
    lengthName = (int)pacote[index++];
    nomeFicheiro = (char*)malloc(lengthName);
    for(int i=index; i<index+lengthName; i++){
      nomeFicheiro[j++]=(char)pacote[i];
    }
  }

  printf("tamanho ficheiro: %ld\n", sizeFicheiro);
  printf("nome do ficheiro: %s\n", nomeFicheiro);
  printf("pacotes a receber: %d\n", packages);

  mensagem=(unsigned char*)malloc(sizeFicheiro);

}

void pacoteDados(unsigned char* pacote, int sizePacote){
  int end = (256 * (int)pacote[2]) + (int)pacote[3];

  int j=4;

  for(int i=ind; i<ind+end; i++){
    mensagem[i]=pacote[j++];
  }
  ind = ind + end;
  return;
}

void ficheiro(){
  FILE *fp;
  fp=fopen("recetor.gif","wb");

  if(fp==NULL){
    printf("Erro na criação de ficheiro\n");
    exit(1);
  }

  fwrite(mensagem, 1, sizeof(unsigned char), fp);

}
