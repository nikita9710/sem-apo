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
//usual blockXrange is 120, usual blockYRange is 64 //53
void blockColorChange(int blockX, int blockY, int blockXrange, int blockYrange,  uint16_t color){
    for(int x = blockX; x < blockX + blockXrange; ++x){
        for(int y = blockY; y < blockY + blockYrange; ++y){
            lcdPixels[x][y] = color;
        }
    }
};
void background(int blockX, int blockY,uint16_t color, int blockXsize){
  for(int x = blockX; x < blockX + blockXsize; ++x){
        for(int y = blockY; y < blockY + 53; ++y){
            lcdPixels[x][y] = color;
        }
    }

}
void chosenBorder(int blockX, int blockY, uint16_t color){
  //up border
  for(int downBorder = 0; downBorder < 119; downBorder++){
  lcdPixels[downBorder+blockX][blockY+1] = color;
  lcdPixels[downBorder+blockX][blockY+2] = color;
  lcdPixels[downBorder+blockX][blockY+3] = color;
  }
  //left border
  for(int leftBorder = 0; leftBorder < 52; leftBorder++){
  lcdPixels[blockX+1][leftBorder+blockY] = color;
  lcdPixels[blockX+2][leftBorder+blockY] = color;
  lcdPixels[blockX+3][leftBorder+blockY] = color;
  }
  //right border
  for(int leftBorder = 0; leftBorder < 52; leftBorder++){
  lcdPixels[blockX+116][leftBorder+blockY] = color;
  lcdPixels[blockX+117][leftBorder+blockY] = color;
  lcdPixels[blockX+118][leftBorder+blockY] = color;
  }
  //down border
  for(int downBorder = 0; downBorder < 119; downBorder++){
  lcdPixels[downBorder+blockX][blockY+49] = color;
  lcdPixels[downBorder+blockX][blockY+50] = color;
  lcdPixels[downBorder+blockX][blockY+51] = color;
  }
}

//Fill the bord with a word
void fillBlock(char* str, int blockX, int blockY, int blockXsize, int blockYsize, int size ){
        size_t length = strlen(str);
        size_t i = 0;
        
        int string_width = 0;
        char c;
        int textX,textY,charWidth, num1, num2;
        //up border of a block
        for(int borderX = 0; borderX<blockX+blockXsize; borderX++){
            lcdPixels[borderX][blockY + blockYsize-1] = 0x0;
        }

        //right border of a block
        for(int borderY = 0; borderY < blockY + blockYsize; borderY++){
            lcdPixels[blockX + blockXsize-1][borderY] = 0x0;
        }
        //calculate the length of the string to put it in the middle of the block
        for(; i<length; i++){
            num1 = str[i] - ' ';
            if(num1 < 0) num1 = 0;
            string_width += font_winFreeSystem14x16.width[num1];
            
        }
	
        //left corner to print the string in the block
        textX = blockX + (blockXsize - string_width*size)/2;
        textY = blockY + 8*3/size;
        while((c = *str++) != 0){
	
            num2 = c - ' ';
            if(num2 < 0) num2 = 0;
            charWidth = font_winFreeSystem14x16.width[num2];
            for(int lineChar = 0; lineChar < 16; lineChar++){
                int bitLineChar  = font_winFreeSystem14x16.bits[num2 * 16 + lineChar];
                for(int xBit = 0; xBit<charWidth; xBit++){
                    if(bitLineChar & 0x8000){
                        for(int k = 0; k <size; k++){
                            for(int l = 0; l<size; l++){
                                lcdPixels[textX + xBit*size +k][textY + lineChar*size +l] = 0x0;

                            }
                        }
                        
                    }
                    bitLineChar = bitLineChar << 1;
                }
            }
            textX= textX+charWidth*size;
	    }      
}

typedef struct 
{
  char r;
  char g;
  char b;
} RGB;

  int toRGB565(char red, char green, char blue)
  {
      uint16_t b = (blue >> 3) & 0x1f;
      uint16_t g = ((green >> 2) & 0x3f) <<5;
      uint16_t r = ((red >> 3) & 0x1f) << 11;

      return (uint16_t) (r | g | b);
      
  }


int rgbtohex(RGB rgb)
{
  return ((rgb.r & 0xff) << 16) + ((rgb.g & 0xff) << 8) + (rgb.b & 0xff);
}

void setfocus(int col, int row, int oldcol, int oldrow)
{
  chosenBorder(120*(oldcol),53*(oldrow-1),0xffff);
  chosenBorder(120*(col),53*(row-1),0x07E0);
}

int main(int argc, char *argv[])
{ 
  unsigned char *mem_base;

  mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);

  if (mem_base == NULL) exit(1);
    //color whole background
    blockColorChange(0,0,480,320, 0xFFFF);
    
    //Fill the  Blocks for two lines
    
    background(0,0,0xD6DA, 120);
    fillBlock("LED:", 0, 0,120,53,2);
    fillBlock("Left", 120, 0,120,53,2);
    fillBlock("Right", 240, 0,120,53,2);
    fillBlock("Both", 360, 0,120,53,2);
    background(0,53,0xD6DA, 120);
    fillBlock("Mode:", 0, 53,120,53,2);
    fillBlock("Still", 120, 53,120,53,2);
    fillBlock("Gradient", 240, 53,120,53,2);
    fillBlock("Blink", 360, 53,120,53,2);
    background(0,106,0xD6DA, 120);
    fillBlock("Color:", 0, 106,120,53,2);
    fillBlock("Use knobs", 120, 106,240,53,2);
    fillBlock("Copy", 360, 106,120,53,2);
    fillBlock("Font", 360, 265,120,53,2);
    background(0,159,0xD6DA,480);
    background(0,212,0xD6DA,480);
    background(0,265,0xD6DA, 360);
    //lcd
    unsigned char *parlcd_mem_base;
    parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
    if (parlcd_mem_base == NULL)  exit(1);;
    parlcd_hx8357_init(parlcd_mem_base);
    *(volatile uint16_t*)(parlcd_mem_base + PARLCD_REG_CMD_o) = 0x2c;


  
  uint32_t rgb_knobs_value;
  int rk, gk, bk,rb, gb, bb;
  clock_t before = clock();
  clock_t beforer = clock();
  clock_t clockCounter = clock();
  clock_t difference = clock() - before;  
  clock_t differencer = clock() - beforer; 
  bool isLeft = false, isRight = false, isTogether = false, changed = true;
  RGB leftcolor = {0, 255, 0}, rightcolor = {255, 0, 0}, leftnew = {0,0,255}, rightnew = {255,255,255};
  int leftcolorhsv = 0xaa11, rightcolorhsv  = 0xbbbb, mode = 1; //1 - still, 2 - gradient, 3 - blinkkink
  int  changetime = 1000000;
  bool reverse = false, lefton = true, righton = true;
  int blinktime = 100000, fadetime = 100000, shift = 50000;
  int currentColumn = 1, prevColumn = 1;
  int currentRow = 1, prevRow = 1;
  while (1)
  {
    rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    bk =  rgb_knobs_value      & 0xFF; // blue knob position
    gk = (rgb_knobs_value>>8)  & 0xFF; // green knob position
    rk = (rgb_knobs_value>>16) & 0xFF; // red knob position
    bb = (rgb_knobs_value>>24) & 1;    // blue button
    gb = (rgb_knobs_value>>25) & 1;    // green button
    rb = (rgb_knobs_value>>26) & 1;    // red buttom
    if(rb && (clock()-clockCounter>=150000)){
      prevColumn = currentColumn;
      prevRow = currentRow;
      switch (mode)
      {
      case 1:
        if (currentRow == 6) break;
        if (currentRow == 3) 
        {
          if (!isTogether) currentColumn = currentColumn == 1 ? 3 : 1;
          break;
        }
        currentColumn = currentColumn == 3 ? 1 : currentColumn+1;
        break;
      case 2:
        if (currentRow == 6) break;
        if (currentRow == 3 && isTogether) 
        {
          currentColumn = currentColumn == 1 ? 2 : 1;
          break;
        }
        currentColumn = currentColumn == 3 ? 1 : currentColumn+1;
        break;
      case 3:
        if (currentRow == 6) break;
        if (currentRow == 3) 
        {
          if (!isTogether) currentColumn = currentColumn == 1 ? 3 : 1;
          break;
        }
        currentColumn = currentColumn == 3 ? 1 : currentColumn+1;
        break;
      }
      setfocus(currentColumn, currentRow, prevColumn, prevRow);
      clockCounter = clock();
    }
    if(gb && (clock()-clockCounter>=150000)){
      prevColumn = currentColumn;
      prevRow = currentRow;
      switch (mode)
      {
      case 1:
        switch (currentRow)
        {
        case 2:
          currentColumn = (currentColumn == 3 && !isTogether) ? 3 : 1;
          currentRow++;
          break;
        case 3:
          currentRow = 6;
          currentColumn = 3;
        break;
        case 6:
          currentRow = 1;
        break;
        default:
          currentRow++;
          break;
        }
        break;
      case 2:
        switch (currentRow)
        {
        case 2:
          if (currentColumn == 3 && isTogether) currentColumn = 2;
          currentRow++;
          break;
        case 4:
          currentRow = 6;
          currentColumn = 3;
        break;
        case 6:
          currentRow = 1;
        break;
        default:
          currentRow++;
          break;
        }
        break;
      case 3:
        switch (currentRow)
        {
        case 2:
          currentColumn = (currentColumn == 3 && !isTogether) ? 3 : 1;
          currentRow++;
          break;
        case 5:
          currentRow = 6;
          currentColumn = 3;
        break;
        case 6:
          currentRow = 1;
        break;
        default:
          currentRow++;
          break;
        }
        break;
      }
      setfocus(currentColumn, currentRow, prevColumn, prevRow);
      clockCounter = clock();
    }
    if(bb && (clock()-clockCounter>=150000)){
      switch (currentRow)
      {
      case 1:
        
        break;
      case 2:
        switch (currentColumn)
        {
        case 1:
          mode = 1;
          background(120,106,0xffff,360);
          background(0,159,0xD6DA,480);
          background(0,212,0xD6DA,480);
          fillBlock("Use knobs", 120, 106,240,53,2);
          if (!isTogether) fillBlock("Copy", 360, 106,120,53,2);
          else background(360,106,0xD6DA, 120);
          break;
        case 2:
          mode = 2;
          background(120,106,0xffff,360);
          background(0,159,0xffff,480);
          background(0,212,0xD6DA,480);
          fillBlock("Color 1", 120, 106,120,53,2);
          fillBlock("Color 2", 240, 106,120,53,2);
          if (!isTogether) fillBlock("Copy", 360, 106,120,53,2);
          else background(360,106,0xD6DA, 120);
          background(0,159,0xD6DA, 120);
          fillBlock("Duration", 0, 159,120,53,2);
          fillBlock("5,0", 120, 159,120,53,2);
          fillBlock("+0,5", 240, 159,120,53,2);
          fillBlock("-0,5", 360, 159,120,53,2);
          break;
        case 3:
          mode = 3;
          background(120,106,0xffff,360);
          background(0,159,0xffff,480);
          background(0,212,0xffff,480);
          fillBlock("Use knobs", 120, 106,240,53,2);
          if (!isTogether) fillBlock("Copy", 360, 106,120,53,2);
          else background(360,106,0xD6DA, 120);
          background(0,159,0xD6DA, 120);
          fillBlock("Setup:", 0, 159,120,53,2);
          fillBlock("Time on", 120, 159,120,53,2);
          fillBlock("Time off", 240, 159,120,53,2);
          fillBlock("Shift", 360, 159,120,53,2);
          background(0,212,0xD6DA, 120);
          fillBlock("Time on", 0, 212,120,53,2);
          fillBlock("2,0", 120, 212,120,53,2);
          fillBlock("+0,5", 240, 212,120,53,2);
          fillBlock("-0,5", 360, 212,120,53,2);
          break;
        }
        break;
      default:
        break;
      }
    }
    // if(rb && (clock()-clockCounter>=200000)){
    //   if(columnCounter>3){
    //      columnCounter = columnCounter - 3;
    //   }
    //   if(columnCounter>=2){
    //     chosenBorder(120*(columnCounter-1),64*rowCounter,0xffff);
    //   }
    //   if(columnCounter==1){
    //     chosenBorder(120*(3),64*rowCounter,0xffff);
    //   }
      
    //   chosenBorder(120*columnCounter,64*rowCounter,0x07E0);
    //   clockCounter = clock();
    //   columnCounter++;

    // }
    // if(gb && (clock()-clockCounter>=200000)){
    //   clockCounter = clock();
    //   rowCounter++;
    //   if(columnCounter>1){
    //   chosenBorder(120*(columnCounter-1),64*rowCounter,0x07E0);
    //   chosenBorder(120*(columnCounter-1),64*(rowCounter-1),0xffff);
    //   }
    //   else{
    //     chosenBorder(120*(columnCounter),64*rowCounter,0x07E0);
    //     chosenBorder(120*columnCounter,64*(rowCounter-1),0xffff);

    //   }


    // }
    // if(bb){
    //   background(240,64,0x7BE0);
    //   chosenBorder(240,64,0x07E0);

    // }

    

    for (int x = 0; x < 320; ++x) {
		  for (int y = 0; y < 480; ++y) {
			  parlcd_write_data(parlcd_mem_base, lcdPixels[y][x]);
		  }
    }
    // if (!changed) continue;
    if (mode == 1)
    {
      *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = leftcolorhsv;
      *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether ? leftcolorhsv : rightcolorhsv;
    }
    else if (mode == 2)
    {
      if (changed) 
      {
        before = clock();
        reverse = false;
        *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(leftcolor);
        *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? rgbtohex(leftcolor) : rgbtohex(rightcolor);
      }
      else
      {
        difference = clock() - before;
        if (difference >= changetime)
        {
          // if (reverse)
          // {
          //   *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(leftcolor);
          //   *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? rgbtohex(leftcolor) : rgbtohex(rightcolor);
          // }
          // else
          // {
          //   *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(leftnew);
          //   *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? rgbtohex(leftnew) : rgbtohex(rightnew);
          // }
          // reverse = !reverse;
          before = clock();
          char tempr = leftcolor.r, tempg = leftcolor.g, tempb = leftcolor.b;
          leftcolor.r = leftnew.r;
          leftcolor.g = leftnew.g;
          leftcolor.b = leftnew.b;
          leftnew.r = tempr;
          leftnew.g = tempg;
          leftnew.b = tempb;
          tempr = rightcolor.r, tempg = rightcolor.g, tempb = rightcolor.b;
          rightcolor.r = rightnew.r;
          rightcolor.g = rightnew.g;
          rightcolor.b = rightnew.b;
          rightnew.r = tempr;
          rightnew.g = tempg;
          rightnew.b = tempb;
        }
        else
        {
          RGB t = {leftcolor.r + ((double)difference)/changetime*(signed char)(leftnew.r - leftcolor.r), 
              leftcolor.g + ((double)difference)/changetime*(leftnew.g - leftcolor.g), 
              leftcolor.b + ((double)difference)/changetime*(leftnew.b - leftcolor.b)};
            *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(t);
            if (!isTogether)
            {
              t.r = rightcolor.r + ((double)difference)/changetime*(rightnew.r - rightcolor.r);
              t.g = rightcolor.g + ((double)difference)/changetime*(rightnew.g - rightcolor.g);
              t.b = rightcolor.b + ((double)difference)/changetime*(rightnew.b - rightcolor.b);
            }
            *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = rgbtohex(t);
        }
      }
    }
    else
    {
      if (changed) 
      {
        before = clock();
        beforer = clock() - shift;
        lefton = true;
        righton = true;
        *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = leftcolorhsv;
        *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? leftcolorhsv : rightcolorhsv;
      }
      else
      {
        difference = clock() - before;
        int differencer = clock() - beforer;
        if (lefton)
        {
          if (difference >= blinktime)
          {
            *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = 0x0;
            if (shift == 0) 
            {
              *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = 0x0;
              righton = false;
            }
            lefton = false;
            before = clock();
          }
        }
        else
        {
          if (difference >= fadetime)
          {
            *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = leftcolorhsv;
            if (shift == 0) 
            {
              *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? leftcolorhsv : rightcolorhsv;
              righton = true;
            }
            lefton = true;
            before = clock();
          }
        }
        if (shift != 0)
        {
            if (righton)
            {
              if (differencer >= blinktime)
              {
                *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = 0x0;
                righton = false;
                beforer = clock();
              }
            }
            else
            {
              if (differencer >= fadetime)
              {
                *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? leftcolorhsv : rightcolorhsv;
                righton = true;
                beforer = clock();
              }
            }
        }
      }
    }
    changed = false;
  }
  


  return 0;
}

