#include "client.h"

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

#define MAXSIZE 128

int readServer(int sockfd){
  FILE* fp = fdopen(sockfd, "r");
	char* buff;
	size_t bytes = 0;
	char response[3];

	while(getline(&buff, &bytes, fp) > 0){
		printf("< %s", buff);

		if(buff[3] == ' '){
			for(int i=0; i<3; i++) response[i]=buff[i];
			break;
		}
	}
  int code = atoi(response);
  return code;
}

int readServerPassive(int sockfd, int *port){
  FILE* fp = fdopen(sockfd, "r");
	char* buff;
	size_t bytes = 0;
	char response[3];

  char bit[3];
  int state=0;
  int ip[6];
  int ind_bit=0, ind_ip=0;

	while(getline(&buff, &bytes, fp) > 0){
		printf("< %s", buff);

		if(buff[3] == ' '){
      for(int i=0; i<3; i++){
         response[i]=buff[i];
      }

      for(int i=0; i<strlen(buff); i++){
        if(buff[i]=='('){
          state = 1;
        }
        else if(buff[i]==','){
          ip[ind_ip++]=atoi(bit);
          ind_bit=0;
          memset(bit,0,3);
        }
        else if(buff[i]==')'){
          ip[ind_ip++]=atoi(bit);
          break;
        }
        else if(state==1) bit[ind_bit++]=buff[i];

      }

			break;
		}
	}
  (*port)=ip[4]*256+ip[5];
  int code = atoi(response);
  return code;
}

int writeServer(int sockfd, char* comando, char* path){
  char *send = (char*) malloc((strlen(comando)+strlen(path))*sizeof(char));
  strcat(send,comando);
  strcat(send,path);
  send[strlen(send)]='\n';
  printf("%s", send);

  int res = write(sockfd, send, strlen(send));
  return res;
}
