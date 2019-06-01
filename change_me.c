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

int main(int argc, char *argv[])
{
  printf("Hello world\n");
  unsigned char *mem_base;

  mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);

  if (mem_base == NULL) exit(1);
  *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = 0x00FF0000;
  *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = 0x000000FF; 

  unsigned char *parlcd_mem_base;

  parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);


  if (parlcd_mem_base == NULL)  exit(1);

  parlcd_hx8357_init(parlcd_mem_base);

  *(volatile uint16_t*)(parlcd_mem_base + PARLCD_REG_CMD_o) = 0x2c;

  uint32_t rgb_knobs_value;
  int rk, gk, bk,rb, gb, bb;
  clock_t before = clock();
  clock_t difference = clock() - before;
<<<<<<< HEAD
  typedef struct Point{
    int x;
    int y;
  }

//function to find the left corner for a text
  // Point textCornerCoordinate(char* str, int blockX, int blockY){
  //   int string_width = 0;
  //   char c;
  //   while((c = *str++) != 0){
  //     int cix = c - ' ';
  //     if(cix < 0) cix = 0;
  //     string_width += font_winFreeSystem14x16.width[cix];
  //   }
  //   Point coor;
  //   coor.x = blockX + (120 - string_width)/2;
  //   coor.y = blockY + 8;
  //   return coor

  // }

  // fillBlock(char *str, int coorX, int coorY){
  int string_width = 0;
  char c;
  int textX;
  int textY;
  while((c = *str++) != 0){
    int num = c - ' ';
    if(num < 0) num = 0;
    string_width += font_winFreeSystem14x16.width[num];
    printf(font_winFreeSystem14x16.width[num]);
  }
  textX = coorX + (120 - string_width)/2;
  textY = coorY + 8;

  // }
  //fill the block (coordinates of a block and etc)
  //fillBlock(char* str, int  blockX, int  blockY){textCoor = textCornerCoordinate(char*str, )
  //for char in a string
  //pixels aldy , pixeldyn tuse almastyru
  //дефолтные по бокам ,12 вызовов для функции
  //по нажатию кноба отслеживать их координаты 
  //наверху название 
  //внизу подсказки
  //передавать я буду 0 1 2 завимисимости от положения кноба
  //передавать значения лефтколор и блаблрала 
  
=======
>>>>>>> master
  while (1)
  {
    rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    bk =  rgb_knobs_value      & 0xFF; // blue knob position
    gk = (rgb_knobs_value>>8)  & 0xFF; // green knob position
    rk = (rgb_knobs_value>>16) & 0xFF; // red knob position
    bb = (rgb_knobs_value>>24) & 1;    // blue button
    gb = (rgb_knobs_value>>25) & 1;    // green button
    rb = (rgb_knobs_value>>26) & 1;    // red buttom


  bool isTogether = false;
  int leftcolor, rightcolor, mode; //0 - still, 1 - gradient, 2 - blinkkink
  int leftnew, rightnew, changetime;
  int blinktime, faddetime, phase;

  before = clock();
  difference = clock() - before;
    // int r = bk;
    // int x0 = gk;
    // int y0 = rk;
    // for (int i = 0; i < 320*480; i++)
    // {
    //   int x = i / 480 -x0;
    //   int y = i % 480 -y0;
    //   if (abs(x*x+y*y - r*r) <= 2*r)
    //   {
    //     *(volatile uint16_t*)(parlcd_mem_base + PARLCD_REG_DATA_o) = 0x0;
    //     //usleep(10);
    //   }
    //   else
    //   {
    //     *(volatile uint16_t*)(parlcd_mem_base + PARLCD_REG_DATA_o) = 0xFFFF;
    //     //usleep(10);
    //   }
    // }
    // usleep(500);
    // // printf("%d %d %d\n", bk, gk, rk);
  }


  printf("Goodbye world\n");

  return 0;
}
