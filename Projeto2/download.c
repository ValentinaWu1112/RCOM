#include "client.h"

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

/*ftp://[anonymous:anonymous@]ftp.up.pt/pub/kodi/timestamp.txt*/

#define SERVER_PORT 21

#define MAXSIZE 128

char* user;
char* password;
char* host;
char* url_path;
char* filename;

/*Funções*/
void getInformation(char *arg);
void getFileName(char *path);

int main(int argc, char  *argv[]) {

  if(argc<2){
    printf("Usage: ftp://[<user>:<password>@]<host>/<url-path>\n");
    exit(1);
  }

  /*TCP socket*/
  int sockfd;
  struct sockaddr_in server_addr;

  /*Host Name*/
  struct hostent *h;

  getInformation(argv[1]);

  printf("User: %s\n", user);
  printf("Password: %s\n", password);
  printf("Host: %s\n", host);
  printf("Path: %s\n", url_path);

  getFileName(url_path);
  printf("File Name: %s\n", filename);

  //-------------------------------------------

  /*Host*/
  if ((h = gethostbyname(host)) == NULL) {
    herror("gethostbyname()");
    exit(-1);
  }
  printf("Host name  : %s\n", h->h_name);
  printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *) h->h_addr)));

  //------------------------------------------------
  /*server address handling*/
  bzero((char *) &server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *) h->h_addr)));    /*32 bit Internet address network byte ordered*/
  server_addr.sin_port = htons(SERVER_PORT);        /*server TCP port must be network byte ordered */

  /*open a TCP socket*/
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket()");
    exit(-1);
  }

  /*connect to the server*/
  if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    perror("connect()");
    exit(-1);
  }

  int code;

  /*Estabelimento da coneccao com o servidor*/
  code = readServer(sockfd);

  /*Enviar user e resposta*/
  if(code==OKAY) {
    writeServer(sockfd, "user ", user);
    code = readServer(sockfd);
  }
  else {
    perror("Conecção estabelicida sem sucesso\n");
  }

  /*Enviar pass e resposta*/
  if(code==USER){
    writeServer(sockfd, "pass ", password);
    code = readServer(sockfd);
  }
  else{
    perror("Erro no user");
  }

  int porta;
  if(code==PASS){
    writeServer(sockfd, "pasv ", "");
    code = readServerPassive(sockfd,&porta);
    printf("porta: %d\n", porta);
  }

  writeServer(sockfd, "quit ", "");
  code = readServer(sockfd);

  if(code==QUIT) close(sockfd);

  return 0;
}

void getInformation(char *arg){
  /*URL*/
  user=(char*)malloc(MAXSIZE*sizeof(char));
  password=(char*)malloc(MAXSIZE*sizeof(char));
  host=(char*)malloc(MAXSIZE*sizeof(char));
  url_path=(char*)malloc(MAXSIZE*sizeof(char));

  int index=0;
  int length = strlen(arg);

  while(arg[index]!='['){
    index++;
  }
  index++;

  /*user*/
  int i=0;
  while(arg[index]!=':'){
    user[i++]=arg[index];
    index++;
  }
  index++;

  /*password*/
  i=0;
  while(arg[index]!='@'){
    password[i++]=arg[index];
    index++;
  }
  index++;

  while(arg[index]!=']') index++;
  index++;

  /*host*/
  i=0;
  while(arg[index]!='/'){
    host[i++]=arg[index];
    index++;
  }
  index++;

  /*url-path*/
  i=0;
  while (index<length) {
    url_path[i++]=arg[index];
    index++;
  }
}

void getFileName(char *path){
  filename=(char*)malloc(MAXSIZE*sizeof(char));

  char *token;
  token=strtok(path,"/");
  while (token!=NULL) {
    memcpy(filename,token, strlen(token)+1);
    token=strtok(NULL,"/");
  }
}
