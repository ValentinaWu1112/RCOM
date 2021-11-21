#include "EAplicacao.h"
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

#define M 0xFF

unsigned char N = 0x00;

/*Variáveis para os ficheiros*/
unsigned char* nomeFicheiro;
off_t sizeFicheiro;
int sizeNomeFicheiro;
unsigned char* conteudo;

void ficheiro(char *file){
  sizeNomeFicheiro = strlen(file);
  nomeFicheiro = (unsigned char*) malloc(sizeNomeFicheiro);
  nomeFicheiro = (unsigned char*) file;

  struct stat s;

  if (stat(file, &s) == -1) {
    perror("stat");
    exit(EXIT_FAILURE);
  }

  sizeFicheiro = s.st_size;
  long int tamanho = (long int)sizeFicheiro;

  conteudo = (unsigned char*) malloc(tamanho);

  FILE *fp;
  fp = fopen(file, "rb");

  if(fp==NULL){
    printf("Ficheiro não existe\n");
    exit(1);
  }

  int ind=0;
  unsigned char c;
  while(!feof(fp)){
    c = fgetc(fp);
    conteudo[ind]=c;
    ind++;
  }


  FILE *fd;
  fd = fopen("emissor.txt", "wb");
  for(int i=0; i<(long int)sizeFicheiro; i++){
    fprintf(fd, "%u ", conteudo[i]);
  }
  printf("sizeFicheiro: %lu\n", sizeFicheiro);
  /*printf("NomeFicheiro: \n");
  for(int i=0; i<sizeNomeFicheiro; i++){
    printf("%u ", nomeFicheiro[i]);
  }printf("\n");

  printf("conteudo do ficheiro\n");
  for(int i=0; i<tamanho; i++){
    printf("%u ", conteudo[i]);
  }printf("\n");*/

}

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

int Campos(off_t size, int* resto){
  long int tmp = (long int) size;
  int r=1;
  while(tmp-255 > 0){
    r++;
    tmp-=255;
  }
  *resto = tmp;
  return r;
}


int juntarV1(unsigned char* pacote, int start, int l1, int resto){
  int i;
  for(i=start; i<(start+l1-1); i++){
    pacote[i]=M;
  }
  pacote[i++]=(unsigned char)resto;
  return i;
}

unsigned char* criarPacoteControlo(unsigned char controlo, int *sizePacote, int *pacoteEnviar, int *l){
  unsigned char* pacote;
  int resto, i=0;
  int l1 = Campos(sizeFicheiro, &resto);

  pacote = (unsigned char*)malloc(5+l1+sizeNomeFicheiro);

  pacote[i++]=controlo;
  pacote[i++]=T1;
  pacote[i++]=(unsigned char)l1;
  i = juntarV1(pacote,i,l1,resto);
  pacote[i++]=T2;
  pacote[i++]=(unsigned char)sizeNomeFicheiro;
  memcpy(pacote+i, nomeFicheiro, sizeNomeFicheiro);
  i=i+sizeNomeFicheiro;

  *l = resto;
  *sizePacote=i;
  *pacoteEnviar=l1;

  return pacote;
}

void emissor(int fd){
  unsigned char* pacote;
  int sizePacote, pacoteEnviar, l1;

  pacote = criarPacoteControlo(START, &sizePacote, &pacoteEnviar, &l1);

  int res=llwrite(fd,pacote,sizePacote);
  if(res>0){ //START
    printf("Trama START enviada com sucesso, com %d bits\n", res);
  }
  else{
    printf("Trama START enviada sem sucesso\n");
    exit(1);
  }

  int start=0, end=256;
  while(pacoteEnviar>0){
    if(pacoteEnviar==1){
      pacote= criarPacoteDados(conteudo, start, (long int)sizeFicheiro, 0x00, (unsigned char)l1, &sizePacote);
      res=llwrite(fd,pacote,sizePacote);
      if(res>0){
        printf("Trama I %d enviada com sucesso, com %d bits\n",(int)pacote[1], res);
      }
    }
    else{
      pacote= criarPacoteDados(conteudo, start, end, 0x01, 0x00, &sizePacote);
      res=llwrite(fd,pacote,sizePacote);
      if(res>0){
        printf("Trama I %d enviada com sucesso, com %d bits\n",(int)pacote[1] ,res);
      }
      start=end;
      end+=256;
    }
    pacoteEnviar--;
  }

  pacote = criarPacoteControlo(END, &sizePacote, &pacoteEnviar, &l1);
  res=llwrite(fd,pacote,sizePacote);
  if(res>0){ //START
    printf("Trama END enviada com sucesso, com %d bits\n", res);
  }
  else{
    printf("Trama END enviada sem sucesso\n");
    exit(1);
  }
}

unsigned char* criarPacoteDados(unsigned char* mensagem, int start,long int end, unsigned char l2, unsigned char l1, int *sizePacote){
  unsigned char* pacote;
  int k = (256 * (int)l2) + (int)l1;
  pacote = (unsigned char*) malloc (4+k);

  int i =0;
  pacote[i++]=DADOS;
  pacote[i++]=N;
  pacote[i++]=l2;
  pacote[i++]=l1;

  for(int j=start; j<end; j++){
    pacote[i++]=mensagem[j];
  }

  *sizePacote=i;
  N+=0x01;
  return pacote;
}
