#define _POSIX_C_SOURCE 200112L
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "font_prop14x16.c"
#include <string.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#define LOCAL_PORT 8888
#define TARGET_PORT 5555
#define TARGET_IP "147.32.216.183"

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

char buffer_rx[20],charReflector[1], charMode[1],charRedLeft[3], charGreenLeft[3], charBlueLeft[3], charRedRight[3],charGreenRight[3],charBlueRight[3];
int reflector[1], mode[1];
RGB colorLeft, colorRight;


int main() 
{ 
    //create socket and connect 
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
    
    //initialize knobs and leds
    unsigned char *mem_base;
    mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
    if (mem_base == NULL) exit(1);

    recvfrom(sockfd, buffer_rx, 20, 0, (struct sockaddr*)&from, &fromlen);

    memset(charReflector, 0, 1);
	memcpy(charReflector, buffer_rx, 1);
	reflector = atoi(charReflector);

    memset(charMode, 0, 1);
	memcpy(charMode, buffer_rx+1, 1);
	mode = atoi(charMode);

    memset(charRedLeft, 0, 3);
	memcpy(charRedLeft, buffer_rx + 2, 3);
	colorLeft.r = atoi(charRedLeft);

    memset(charGreenLeft, 0, 3);
	memcpy(charGreenLeft, buffer_rx + 5, 3);
	colorLeft.g = atoi(charGreenLeft);
    
    memset(charBlueLeft, 0, 3);
	memcpy(charBlueLeft, buffer_rx + 8, 3);
	colorLeft.b = atoi(charBlueLeft);

    memset(charRedRight, 0, 3);
	memcpy(charRedRight, buffer_rx + 11, 3);
	colorRight.r = atoi(charRedRight);

    memset(charGreenRight, 0, 3);
	memcpy(charGreenRight, buffer_rx + 14, 3);
	colorRight.g = atoi(charGreenRight);
    
    memset(charBlueRight, 0, 3);
	memcpy(charBlueRight, buffer_rx + 17, 3);
	colorRight.b = atoi(charBlueRight);

    if (mode == 1)
    {
      *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&colorLeft);
      *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = rgbtohex(&colorRight);
    }

    closesocket(sockfd);
}
