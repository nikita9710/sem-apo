#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "font_prop14x16.c"
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LOCAL_PORT 8888
#define TARGET_PORT 8888
#define TARGET_IP "192.168.202.208"

char  buffer_rx[20], strReflector[1], strMode[1], redLeft[3],greenLeft[3], blueLeft[3], redRight[3], greenRight[3], blueRight[3];

typedef struct 
{
  char r;
  char g;
  char b;
} RGB;


int rgbtohex(RGB* rgb)
{
  return ((rgb->r & 0xff) << 16) + ((rgb->g & 0xff) << 8) + (rgb->b & 0xff);
}


int main(int argc, char *argv[])
{ 
  
  int sockfd;
  struct sockaddr_in local;
  struct sockaddr_in from;
  int fromlen = sizeof(from);
  local.sin_family = AF_INET;
  local.sin_port = htons(LOCAL_PORT);
  local.sin_addr.s_addr = INADDR_ANY;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (bind(sockfd, (struct sockaddr*)& local, sizeof(local)) != 0) {
  printf("Binding error!\n");
  getchar();
  return 1;}

  struct sockaddr_in addrDest;
  addrDest.sin_family = AF_INET;
  addrDest.sin_port = htons(TARGET_PORT);
  inet_pton(AF_INET, (TARGET_IP), &addrDest.sin_addr.s_addr);


//   sprintf(strMode, "%d",mode);
//   sprintf(strReflector, "%d",mode);
  sprintf(strReflector, "%d",2);
  sprintf(strMode, "%d",1);
  sprintf(redLeft, "%d", 180);
  sprintf(greenLeft, "%d", 12);
  sprintf(blueLeft, "%d", 175);
  sprintf(redRight, "%d", 175);
  sprintf(greenRight, "%d", 175);
  sprintf(blueRight, "%d", 175);
  sprintf(isTogether, "%d",1);

  strcat(buffer_rx, strReflector);
  strcat(buffer_rx, strMode);
  strcat(buffer_rx, redLeft);
  strcat(buffer_rx, greenLeft);
  strcat(buffer_rx, blueLeft);
  strcat(buffer_rx, redRight);
  strcat(buffer_rx, greenRight);
  strcat(buffer_rx, blueRight);

  sendto(sockfd, buffer_rx, sizeof(buffer_rx), 0, (struct sockaddr*)& addrDest, sizeof(addrDest));
  printf("SENT\n");

  printf("\nMY JOB IS DONE");
  close(sockfd);


  return 0;
}
