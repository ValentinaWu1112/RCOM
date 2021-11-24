#ifndef EAPLICACAO_H
#define EAPLICACAO_H

#include <sys/stat.h>

/*Pacotes de dados*/
#define DADOS 0x01

/*Pacotes de controlo*/
#define START 0x02
#define END 0x03
#define T1 0x00 //tamanho do ficheiro
#define T2 0x01 //nome do ficheiro
/*#define L -> tamanho de campos de v
#define V -> valor do parametro (T)
*/

unsigned char* criarPacoteControlo(unsigned char controlo, int *sizePacote);
unsigned char* criarPacoteDados(unsigned char* mensagem, int start, long int end, unsigned char l2, unsigned char l1, int *sizePacote);
void ficheiro(char *file);
void call_llopen(int fd);
void call_llclose(int fd);
void emissor(int fd);

#endif
