#ifndef CLIENT_H
#define CLIENT_H

#define OKAY 220 //ligar ao servidor
#define USER 331 //specify password
#define PASS 230 //Login successful
#define PASSIVE 227 //Passive Mode
#define RETR 150 //retr -> opening Binary Mode
#define TRANSF 226 //transfer complete
#define QUIT 221 //quit successful


int readServer(int sockfd);
int writeServer(int sockfd, char* comando, char* path);
int readServerPassive(int sockfd, int *port);
void readServerFile(int sockfd, char* filename);

#endif
