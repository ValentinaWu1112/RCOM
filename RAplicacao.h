#ifndef RAPLICACAO_H
#define RAPLICACAO_H

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

void call_llopen(int fd);
void call_llclose(int fd);
void recetor(int fd);
int analisarPacote(unsigned char* pacote, int sizePacote);
void pacoteStart(unsigned char* pacote, int sizePacote);
void pacoteDados(unsigned char* pacote, int sizePacote);
void ficheiro();

#endif
