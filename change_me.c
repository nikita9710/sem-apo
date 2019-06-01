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

void boardBorder(){
    //down border
    for(int downBorder = 0; downBorder < 480; downBorder++){
        lcdPixels[downBorder][319] = 0xF800;
	    lcdPixels[downBorder][318] = 0xF800;
	    lcdPixels[downBorder][317] = 0xF800;
    }
    //left border
    for(int leftBorder = 0; leftBorder < 320; leftBorder++){
        lcdPixels[0][leftBorder] = 0xF800;
	    lcdPixels[1][leftBorder] = 0xF800;
	    lcdPixels[2][leftBorder] = 0xF800;
    }
    //right border
    for(int leftBorder = 0; leftBorder < 320; leftBorder++){
        lcdPixels[479][leftBorder] = 0xF800;
	    lcdPixels[478][leftBorder] = 0xF800;
	    lcdPixels[477][leftBorder] = 0xF800;
    }
    //up border
    for(int downBorder = 0; downBorder < 480; downBorder++){
        lcdPixels[downBorder][0] = 0xF800;
	    lcdPixels[downBorder][1] = 0xF800;
	    lcdPixels[downBorder][2] = 0xF800;
    }
}

//Fill the bord with a word
void fillBlock(char* str, int blockX, int blockY, int size){
        size_t length = strlen(str);
        size_t i = 0;
        
        int string_width = 0;
        char c;
        int textX,textY,charWidth, num1, num2;
        //up border of a block
        for(int borderX = 0; borderX<blockX+120; borderX++){
            lcdPixels[borderX][blockY + 63] = 0x0;
        }

        //right border of a block
        for(int borderY = 0; borderY < blockY + 64; borderY++){
            lcdPixels[blockX + 119][borderY] = 0x0;
        }
        //calculate the length of the string to put it in the middle of the block
        for(; i<length; i++){
            num1 = str[i] - ' ';
            if(num1 < 0) num1 = 0;
            string_width += font_winFreeSystem14x16.width[num1];
            
        }
	
        //left corner to print the string in the block
        textX = blockX + (120 - string_width*size)/2;
        textY = blockY + 8*3/size;
        while((c = *str++) != 0){
	
            num2 = c - ' ';
            if(num2 < 0) num2 = 0;
            charWidth = font_winFreeSystem14x16.width[num2];
            for(int lineChar = 0; lineChar < 16; lineChar++){
                int newY = size*lineChar;
                int bitLineChar  = font_winFreeSystem14x16.bits[num2 * 16 + lineChar];
                for(int xBit = 0; xBit<charWidth; xBit++){
                    int newX = size * xBit;
                    if(bitLineChar & 0x8000){
                        for(int k = 0; k <size; k++){
                            for(int l = 0; l<size; l++){
                                lcdPixels[textX + newX +k][textY + newY +l] = 0x0;

                            }
                        }
                        
                    }
                    bitLineChar = bitLineChar << 1;
                }
            }
            textX= textX+charWidth*size;
	    }      
}

int main(int argc, char *argv[])
{ 
    //color whole background
    blockColorChange(0,0,480,320, 0xFFFF);
    
    //Fill the  Blocks for two lines
    fillBlock("Light:", 0, 0,2);
    fillBlock("Left", 120, 0,2);
    fillBlock("Right", 240, 0,3);
    fillBlock("Both", 360, 0,2);
    fillBlock("Mode:", 0, 64,2);
    fillBlock("Still", 120, 64,1);
    fillBlock("Grad", 240, 64,2);
    fillBlock("Shift", 360, 64,3);
    boardBorder();
    //lcd
    unsigned char *parlcd_mem_base;
    parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
    if (parlcd_mem_base == NULL)  exit(1);
    parlcd_hx8357_init(parlcd_mem_base);
    *(volatile uint16_t*)(parlcd_mem_base + PARLCD_REG_CMD_o) = 0x2c;
    // lcd = map_phys_address(PARLCD_REG_BASE_PHYS, 4, false);
    for (int x = 0; x < 320; ++x) {
		for (int y = 0; y < 480; ++y) {
			parlcd_write_data(parlcd_mem_base, lcdPixels[y][x]);
		}
	}

    // int row, columns;
    // for (int row=0; row<320; row++)
    // {
    //     for(int columns=0; columns<480; columns++)
    //         {
    //         printf("%d     ", lcdPixels[columns][row]);
    //         }
    //     printf("\n");
    // }


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


  return 0;
}

