#include "EAplicacao.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

int fd;

int main(int argc, char *argv[]) {

  if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS10\n");
      exit(-1);
  }
  else if (argc < 3){
    printf("Falta de documento para transmitir\n");
    exit(-1);
  }

  fd = open(argv[1], O_RDWR | O_NOCTTY );
  if (fd <0) {perror(argv[1]); exit(-1); }

  call_llopen(fd);

  ficheiro(argv[2]);

  emissor(fd);

  call_llclose(fd);

  return 0;
}
