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
uint16_t lcdPixels[480][320];

//to color one bloch hen it is chosen
//usual blockXrange is 120, usual blockYRange is 64
void blockColorChange(int blockX, int blockY, int blockXrange, int blockYrange,  uint16_t color){
    for(int x = blockX; x < blockX + blockXrange; ++x){
        for(int y = blockY; y < blockY + blockYrange; ++y){
            lcdPixels[x][y] = color;
        }
    }
};

//Fill the bord with a word
void fillBlock(char *str, int blockX, int blockY){
        int string_width = 0;
        char c;
        int textX,textY,charWidth, num1, num2;
        //calculate the length of the string to put it in the middle of the block
        while((c = *str++) != 0){
            num1 = c - ' ';
            if(num1 < 0) num1 = 0;
            string_width += font_winFreeSystem14x16.width[num1];
        }
        //left corner to print the string in the block
        textX = blockX + (120 - string_width)/2;
        textY = blockY + 8;
        
        while((c = *str++) != 0){
            num2 = c - ' ';
            if(num2 < 0) num2 = 0;
            charWidth = font_winFreeSystem14x16.width[num2];
            for(int lineChar = 0; lineChar < 16; lineChar++){
                int bitLineChar  = font_winFreeSystem14x16.bits[num2 * 16 + lineChar];
                for(int xBit = 0; xBit<charWidth; xBit++){
                    if(bitLineChar & 0x8000){
                        lcdPixels[textX + xBit][textY + lineChar] = 0x0;
                    }
                    bitLineChar = bitLineChar << 1;
                }
            }
	    }      
}
int main(int argc, char *argv[])
{ 
    //color whole background
    blockColorChange(0,0,480,320, 0xFFFF);
    //Fill the first Block
    fillBlock("hello", 0, 0);

//   printf("Hello world\n");
//   unsigned char *mem_base;

//   mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);

//   if (mem_base == NULL) exit(1);
//   *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = 0x00FF0000;
//   *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = 0x000000FF; 

//   unsigned char *parlcd_mem_base;

//   parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);


//   if (parlcd_mem_base == NULL)  exit(1);

//   parlcd_hx8357_init(parlcd_mem_base);

//   *(volatile uint16_t*)(parlcd_mem_base + PARLCD_REG_CMD_o) = 0x2c;

//   uint32_t rgb_knobs_value;
//   int rk, gk, bk,rb, gb, bb;
//   clock_t before = clock();
//   clock_t difference = clock() - before;  
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
  
//   while (1)
//   {
//     rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
//     bk =  rgb_knobs_value      & 0xFF; // blue knob position
//     gk = (rgb_knobs_value>>8)  & 0xFF; // green knob position
//     rk = (rgb_knobs_value>>16) & 0xFF; // red knob position
//     bb = (rgb_knobs_value>>24) & 1;    // blue button
//     gb = (rgb_knobs_value>>25) & 1;    // green button
//     rb = (rgb_knobs_value>>26) & 1;    // red buttom


//   bool isTogether = false;
//   int leftcolor, rightcolor, mode; //0 - still, 1 - gradient, 2 - blinkkink
//   int leftnew, rightnew, changetime;
//   int blinktime, faddetime, phase;

//   before = clock();
//   difference = clock() - before;
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
//   }


  printf("Goodbye world\n");

  return 0;
}

