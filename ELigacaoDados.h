#ifndef ELIGACAODADOS_H
#define ELIGACAODADOS_H

#include <sys/stat.h>

#define F 0x7E
#define A 0x03

/*Tramas: Campo C*/
/*Tramas de supervisão*/
#define SET 0x03
#define DISC 0x0B
#define UA 0x07
#define RR0 0x05
#define RR1 0x85
#define REJ0 0x01
#define REJ1 0x81
/*Tramas de informação*/
#define C0 0x00
#define C1 0x40

/*Campo de proteção
#define BCC1
#define BCC2
*/

/*Transparência*/
#define SETEE 0x7E
#define SETED 0x7D
#define CINCOE 0x5E
#define CINCOD 0x5D

int llopen(int fd);
void criarTramaSupervisor(unsigned char* trama, unsigned char comando);
int verificarTramaS(int fd, unsigned char comando);
int llclose(int fd);
int llwrite(int fd, unsigned char* pacote, int length);
int llwrite(int fd, unsigned char* pacote, int sizePacote);
unsigned char* stuffing(unsigned char* trama, int length, int *sizeTramaI);
unsigned char* lerTrama(int fd);
unsigned char calculoBCC2(unsigned char* pacote, int sizeP);
#endif
