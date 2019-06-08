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
#ifndef _NETWORK_H_
#define _NETWORK_H_

#define LOCAL_PORT 8888
#define TARGET_PORT 8888
#define TARGET_IP "192.168.202.208"
uint16_t lcdPixels[480][320];

char buffer_rx[20];
char buffer_tx[9];
char red[3];
char green[3];
char blue[3];
//to color one bloch hen it is chosen
//usual blockXrange is 120, usual blockYRange is 64 //53
void blockColorChange(int blockX, int blockY, int blockXrange, int blockYrange,  uint16_t color){
    for(int x = blockX; x < blockX + blockXrange; ++x){
        for(int y = blockY; y < blockY + blockYrange; ++y){
            lcdPixels[x][y] = color;
        }
    }
};
void background(int blockX, int blockY,uint16_t color, int blockXsize, int delta){
  for(int x = blockX + delta; x < blockX + blockXsize - delta; ++x){
        for(int y = blockY + delta; y < blockY + 53 - delta; ++y){
            lcdPixels[x][y] = color;
        }
    }

}
void chosenBorder(int blockX, int blockY, uint16_t color, bool isDoubled, int delta){
  //up border
  if (color == 0xffff && lcdPixels[blockX+3][blockY+3] == 0xA6D5) color = 0xA6D5;
  for(int downBorder = 0+delta; downBorder < (isDoubled ? 239: 119)-delta; downBorder++){
  lcdPixels[downBorder+blockX][blockY+0+delta] = color;
  lcdPixels[downBorder+blockX][blockY+1+delta] = color;
  lcdPixels[downBorder+blockX][blockY+2+delta] = color;
  }
  //left border
  for(int leftBorder = 0+delta; leftBorder < 52-delta; leftBorder++){
  lcdPixels[blockX+0+delta][leftBorder+blockY] = color;
  lcdPixels[blockX+1+delta][leftBorder+blockY] = color;
  lcdPixels[blockX+2+delta][leftBorder+blockY] = color;
  }
  //right border
  for(int leftBorder = 0+delta; leftBorder < 52-delta; leftBorder++){
  lcdPixels[blockX+(isDoubled ? 236: 116)-delta][leftBorder+blockY] = color;
  lcdPixels[blockX+(isDoubled ? 237: 117)-delta][leftBorder+blockY] = color;
  lcdPixels[blockX+(isDoubled ? 238: 118)-delta][leftBorder+blockY] = color;
  }
  //down border
  for(int downBorder = 0+delta; downBorder < (isDoubled ? 239: 119)-delta; downBorder++){
  lcdPixels[downBorder+blockX][blockY+49-delta] = color;
  lcdPixels[downBorder+blockX][blockY+50-delta] = color;
  lcdPixels[downBorder+blockX][blockY+51-delta] = color;
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
        for(int borderX = blockX; borderX<blockX+blockXsize; borderX++){
            lcdPixels[borderX][blockY + blockYsize-1] = 0x0;
        }

        //right border of a block
        for(int borderY = blockY; borderY < blockY + blockYsize; borderY++){
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
        textY = blockY + (53-size*16)/2;
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
                                lcdPixels[textX + xBit*size+k][textY + lineChar*size + l] = 0x0;

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


int rgbtohex(RGB* rgb)
{
  return ((rgb->r & 0xff) << 16) + ((rgb->g & 0xff) << 8) + (rgb->b & 0xff);
}

void setfocus(int col, int row, int oldcol, int oldrow, int mode)
{
  if (oldrow == 3 && oldcol == 1 && mode != 2) chosenBorder(120*(oldcol),53*(oldrow-1),0xffff, true,0);
  else chosenBorder(120*(oldcol),53*(oldrow-1),0xffff, false,0);
  if (row == 3 && col == 1 && mode != 2) chosenBorder(120*(col),53*(row-1),0x07E0, true,0);
  else chosenBorder(120*(col),53*(row-1),0x07E0, false,0);
}

void focus(int col, int row, int mode)
{
  if (row == 3 && col == 1 && mode != 2) chosenBorder(120*(col),53*(row-1),0x07E0, true,0);
  else chosenBorder(120*(col),53*(row-1),0x07E0, false,0);
}

void focusColor(int col, int row)
{
  chosenBorder(120*(col),53*(row-1),0xA6D6, false,2);
}

void updateColors(RGB* leftcolor, RGB* rightcolor,  RGB* leftnew, RGB* rightnew, int mode, bool isLeft, bool isFirst, char fontsize)
{
  background(120,106,0xffff, 240,0);
      if (mode == 2)
      {
          if (isLeft)
          {
            background(120,106,toRGB565(leftcolor->r, leftcolor->g, leftcolor->b), 120,3);
            background(240,106,toRGB565(leftnew->r, leftnew->g, leftnew->b), 120,3);
          }
          else
          {
            background(120,106,toRGB565(rightcolor->r, rightcolor->g, rightcolor->b), 120,3);
            background(240,106,toRGB565(rightnew->r, rightnew->g, rightnew->b), 120,3);
          }
          fillBlock("Color 1", 120, 106,120,53,fontsize);
          fillBlock("Color 2", 240, 106,120,53,fontsize);
          focusColor(isFirst? 1 : 2, 3);
      }
      else
      {
          if (isLeft) background(120,106,toRGB565(leftcolor->r, leftcolor->g, leftcolor->b), 240,3);
          else background(120,106,toRGB565(rightcolor->r, rightcolor->g, rightcolor->b), 240,3);
          fillBlock("Use knobs", 120, 106,240,53,fontsize);
      }
}

void setCopyButton(bool isOn, char fontsize)
{
  if (isOn) 
  {
    background(360,106,0xffff, 120,0);
    fillBlock("Copy", 360, 106,120,53,fontsize);
  }
  else background(360,106,0xD6DA, 120,0);
}


void redrawLEDLine(bool isTogether, bool isLeft, char fontsize)
{
  background(120,0,0xffff,120,0);
  background(240,0,0xffff,120,0);
  background(360,0,0xffff,120,0);
  if (isTogether) background(360,0,0xA6D5,120,0);
  else if (isLeft) background(120,0,0xA6D5,120,0);
  else background(240,0,0xA6D5,120,0);
  fillBlock("Left", 120, 0,120,53,fontsize);
  fillBlock("Right", 240, 0,120,53,fontsize);
  fillBlock("Both", 360, 0,120,53,fontsize); 
}

void redrawModeLine(int mode, char fontsize)
{
  background(120,53,0xffff,120,0);
  background(240,53,0xffff,120,0);
  background(360,53,0xffff,120,0);
  background(120*mode,53,0xA6D5,120,0);
  fillBlock("Still", 120, 53,120,53,fontsize);
  fillBlock("Gradient", 240, 53,120,53,fontsize);
  fillBlock("Blink", 360, 53,120,53,fontsize);  
}

void redrawSetupLine(int mode, int time, int selectedsetup, char fontsize)
{
  switch (mode)
  {
  case 1:
    background(0,159,0xD6DA,480,0);
    break;
  
  case 2:
    background(0,159,0xD6DA, 120,0);
    background(120,159,0xffff,360,0);
    fillBlock("Duration", 0, 159,120,53,fontsize);
    char str[3];
    sprintf(str, "%d,%d", time/10, time%10);
    fillBlock(str, 120, 159,120,53,fontsize); 
    fillBlock("+0,5", 240, 159,120,53,fontsize);
    fillBlock("-0,5", 360, 159,120,53,fontsize);
    break;
  
  case 3:
    background(0,159,0xD6DA, 120,0);
    background(120,159,0xffff,360,0);
    background(120*selectedsetup,159,0xA6D5,120,0);
    fillBlock("Setup:", 0, 159,120,53,fontsize);
    fillBlock("Time on", 120, 159,120,53,fontsize);
    fillBlock("Time off", 240, 159,120,53,fontsize);
    fillBlock("Shift", 360, 159,120,53,fontsize);
    break;
  }
}

void redrawPrefLine(int mode, char* text, int time, char fontsize)
{
  switch (mode)
  {
  case 1:
    background(0,212,0xD6DA,480,0);
    break;
  
  case 2:
    background(0,212,0xD6DA,480,0);
    break;
  
  case 3:
    background(0,212,0xD6DA, 120,0);
    background(120,212,0xffff,360,0);
    fillBlock(text, 0, 212,120,53,fontsize);
    char str[3];
    sprintf(str, "%d,%d", time/10, time%10); 
    fillBlock(str, 120, 212,120,53,fontsize);
    fillBlock("+0,1", 240, 212,120,53,fontsize);
    fillBlock("-0,1", 360, 212,120,53,fontsize);
    break;
  }
}

void redrawConstants(char fontsize)
{
  background(0,0,0xD6DA, 120,0);
  fillBlock("LED:", 0, 0,120,53,fontsize);
  background(0,53,0xD6DA, 120,0);
  fillBlock("Mode:", 0, 53,120,53,fontsize);
  background(0,106,0xD6DA, 120,0);
  fillBlock("Color:", 0, 106,120,53,fontsize);
  background(0,265,0xD6DA, 360,0);
  background(360,265,0xffff, 120,0);
  fillBlock("Font", 360, 265,120,53,fontsize);
}

void redrawAll(bool isTogether, RGB* leftcolor, RGB* rightcolor,  RGB* leftnew, RGB* rightnew, int mode, bool isLeft, bool isFirst, char fontsize, char* text, int time, int selectedsetup)
{
  redrawConstants(fontsize);
  redrawLEDLine(isTogether, isLeft,fontsize);
  redrawModeLine(mode,fontsize);
  updateColors(leftcolor, rightcolor, leftnew, rightnew, mode, isLeft, isFirst,fontsize);
  setCopyButton(true,fontsize);
  redrawSetupLine(mode, time, selectedsetup,fontsize);
  redrawPrefLine(mode, text,time,fontsize);
}

char* getTextForSetup(int setup)
{
  if (setup == 1) return "Time on:";
  if (setup == 2) return "Time off:";
  return "Shift:";
}

int main(int argc, char *argv[])
{ 
  // unsigned char *mem_base;

  // mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);

  // if (mem_base == NULL) exit(1);
  // uint32_t rgb_knobs_value;
  // int rk, gk, bk,rb, gb, bb;
  // clock_t before = 0;
  // clock_t beforer = 0;
  // clock_t clockCounter = clock();
  // clock_t difference = 0;  
  // clock_t differencer = 0; 
  // bool isTogether = false, isLeft = true, changed = true, isFirst = true;
  // RGB leftcolor = {243, 31, 83}, rightcolor = {40, 240, 250}, 
  //     leftnew = {192,56,222}, rightnew = {10,130,200}, t = {0,0,0}, 
  //     gradleft = {0,0,0}, gradright = {0,0,0}, 
  //     gradleftnew = {0,0,0}, gradrightnew = {0,0,0};
  //   rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
  // int rprev = (rgb_knobs_value>>16) & 0xFF, gprev = (rgb_knobs_value>>8)  & 0xFF, bprev = rgb_knobs_value      & 0xFF;
  // int  mode = 2; //1 - still, 2 - gradient, 3 - blinkink
  // int  changetime = 1000000;
  // bool lefton = true, righton = true;
  // int selectedsetup = 1;
  // int blinktime = 1000000, fadetime = 1000000, shift = 500000;
  // int currentColumn = 1, prevColumn = 1;
  // int currentRow = 1, prevRow = 1;
  // int fontsize = 1;
  // unsigned char *parlcd_mem_base;
  // parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
  // if (parlcd_mem_base == NULL)  exit(1);
  // parlcd_hx8357_init(parlcd_mem_base);
  // *(volatile uint16_t*)(parlcd_mem_base + PARLCD_REG_CMD_o) = 0x2c;


  // if (mode == 2) redrawAll(isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isTogether && isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), changetime/100000, selectedsetup);
  // else if (mode == 3)
  // {
  //   if (selectedsetup == 1) redrawAll(isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isTogether && isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), blinktime/100000, selectedsetup);
  //   else if (selectedsetup == 2) redrawAll(isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isTogether && isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), fadetime/100000, selectedsetup);
  //   else redrawAll(isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isTogether && isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), shift/100000, selectedsetup);
  // }
  // else redrawAll(isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isTogether && isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), 0, selectedsetup);
  // focus(currentColumn, currentRow, mode);

  // while (1)
  // {
  //   rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
  //   bk =  rgb_knobs_value      & 0xFF; // blue knob position
  //   gk = (rgb_knobs_value>>8)  & 0xFF; // green knob position
  //   rk = (rgb_knobs_value>>16) & 0xFF; // red knob position
  //   bb = (rgb_knobs_value>>24) & 1;    // blue button
  //   gb = (rgb_knobs_value>>25) & 1;    // green button
  //   rb = (rgb_knobs_value>>26) & 1;    // red buttom
  //   if (rprev != rk || gprev != gk || bprev != bk)
  //   {
  //     int dr = rk-rprev,db = bk-bprev,dg = gk-gprev;
  //     rprev = rk;
  //     gprev = gk;
  //     bprev = bk;
  //     if (isLeft)
  //     {
  //       if (mode == 2 && !isFirst)
  //       {
  //       leftnew.r += dr;
  //       leftnew.g += dg;
  //       leftnew.b += db;
  //       }
  //       else
  //       {
  //       leftcolor.r += dr;
  //       leftcolor.g += dg;
  //       leftcolor.b += db;
  //       }
  //     }
  //     else
  //     {
  //       if (mode == 2 && !isFirst)
  //       {
  //       rightnew.r += dr;
  //       rightnew.g += dg;
  //       rightnew.b += db;
  //       }
  //       else
  //       {
  //       rightcolor.r += dr;
  //       rightcolor.g += dg;
  //       rightcolor.b += db;
  //       }
  //     }
  //     changed = true;
  //     updateColors(&leftcolor, &rightcolor, &leftnew, &rightnew, mode, isTogether && isLeft, isFirst,fontsize);
  //     focus(currentColumn, currentRow, mode);
  //   }


  //   if(rb && (clock()-clockCounter>=150000)){
  //     prevColumn = currentColumn;
  //     prevRow = currentRow;
  //     switch (mode)
  //     {
  //     case 1:
  //       if (currentRow == 6) break;
  //       if (currentRow == 3) break;
  //       currentColumn = currentColumn == 3 ? 1 : currentColumn+1;
  //       break;
  //     case 2:
  //       if (currentRow == 6) break;
  //       if (currentRow == 3 && isTogether) 
  //       {
  //         currentColumn = currentColumn == 1 ? 2 : 1;
  //         break;
  //       }
  //       if (currentRow == 4)
  //       {
  //         currentColumn = currentColumn == 3 ? 2 : 3;
  //         break;
  //       }
  //       currentColumn = currentColumn == 3 ? 1 : currentColumn+1;
  //       break;
  //     case 3:
  //       if (currentRow == 6) break;
  //       if (currentRow == 3) break;
  //       if (currentRow == 5)
  //       {
  //         currentColumn = currentColumn == 3 ? 2 : 3;
  //         break;
  //       }
  //       currentColumn = currentColumn == 3 ? 1 : currentColumn+1;
  //       break;
  //     }
  //     setfocus(currentColumn, currentRow, prevColumn, prevRow, mode);
  //     clockCounter = clock();
  //   }


  //   if(gb && (clock()-clockCounter>=150000)){
  //     prevColumn = currentColumn;
  //     prevRow = currentRow;
  //     switch (mode)
  //     {
  //     case 1:
  //       switch (currentRow)
  //       {
  //       case 2:
  //         if (isTogether)
  //         {
  //           currentRow = 6;
  //           currentColumn = 3;
  //         }
  //         else
  //         {
  //           currentRow = 3;
  //           currentColumn = 3;
  //         }
  //       break;
  //       case 3:
  //         currentRow = 6;
  //         currentColumn = 3;
  //         break;
  //       case 6:
  //         currentRow = 1;
  //       break;
  //       default:
  //         currentRow++;
  //         break;
  //       }
  //       break;
  //     case 2:
  //       switch (currentRow)
  //       {
  //       case 2:
  //         if (currentColumn == 3 && isTogether) currentColumn = 2;
  //         currentRow++;
  //         break;
  //       case 3:
  //         if (currentColumn == 1) currentColumn = 2;
  //         currentRow++;
  //         break;
  //       case 4:
  //         currentRow = 6;
  //         currentColumn = 3;
  //       break;
  //       case 6:
  //         currentRow = 1;
  //       break;
  //       default:
  //         currentRow++;
  //         break;
  //       }
  //       break;
  //     case 3:
  //       switch (currentRow)
  //       {
  //       case 2:
  //         if (isTogether) currentRow = 4;
  //         else
  //         {
  //           currentRow = 3;
  //           currentColumn = 3;
  //         }
  //         break;
  //       case 4:
  //         if (currentColumn == 1) currentColumn = 2;
  //         currentRow++;
  //         break;
  //       case 5:
  //         currentRow = 6;
  //         currentColumn = 3;
  //       break;
  //       case 6:
  //         currentRow = 1;
  //       break;
  //       default:
  //         currentRow++;
  //         break;
  //       }
  //       break;
  //     }
  //     setfocus(currentColumn, currentRow, prevColumn, prevRow, mode);
  //     clockCounter = clock();
  //   }

    
  //   if(bb && (clock()-clockCounter>=150000)){
  //     switch (currentRow)
  //     {
  //     case 1:
  //       changed = true;
  //       switch (currentColumn)
  //       {
  //       case 1:
  //         if (!isLeft || isTogether) changed = true;
  //         isLeft = true;
  //         isTogether = false;
  //         break;
  //       case 2:
  //         if (isLeft || isTogether) changed = true;
  //         isLeft = false;
  //         isTogether = false;
  //         break;
  //       case 3:
  //         if (!isTogether) changed = true;
  //         isTogether = true;
  //         break;
  //       }
  //       redrawLEDLine(isTogether, isLeft,fontsize);
  //       updateColors(&leftcolor, &rightcolor, &leftnew, &rightnew, mode, isTogether && isLeft, isFirst,fontsize);
  //       setCopyButton(!isTogether,fontsize);
  //       focus(currentColumn, currentRow, mode);
  //       break;
  //     case 2:
  //       changed = true;
  //       switch (currentColumn)
  //       {
  //       case 1:
  //         mode = 1;
  //         redrawSetupLine(1,0, selectedsetup,fontsize);
  //         redrawPrefLine(1, "", 0,fontsize);
  //         break;
  //       case 2:
  //         mode = 2;
  //         redrawSetupLine(2,changetime/100000, selectedsetup,fontsize);
  //         redrawPrefLine(2, "", 0,fontsize);
  //         break;
  //       case 3:
  //         mode = 3;
  //         redrawSetupLine(3,0, selectedsetup,fontsize);
  //         redrawPrefLine(3, "Time on:", blinktime/100000,fontsize);
  //         break;
  //       }
  //       redrawModeLine(mode,fontsize);
  //       updateColors(&leftcolor, &rightcolor,  &leftnew, &rightnew, mode, isTogether && isLeft, isFirst,fontsize);
  //       break;
  //     case 3:
  //       changed = true;
  //       if (mode == 2)
  //       {
  //           switch (currentColumn)
  //           {
  //             case 1:
  //               isFirst = true;
  //             break;
  //             case 2:
  //               isFirst = false;
  //             break;
  //             case 3:
  //               if (isLeft)
  //               {
  //                 leftcolor.r = rightcolor.r;
  //                 leftcolor.g = rightcolor.g;
  //                 leftcolor.b = rightcolor.b;
  //                 leftnew.r = rightnew.r;
  //                 leftnew.g = rightnew.g;
  //                 leftnew.b = rightnew.b;
  //               }
  //               else
  //               {
  //                 rightcolor.r = leftcolor.r;
  //                 rightcolor.g = leftcolor.g;
  //                 rightcolor.b = leftcolor.b;
  //                 rightnew.r = leftnew.r;
  //                 rightnew.g = leftnew.g;
  //                 rightnew.b = leftnew.b;
  //               }
  //             break;
  //           }
  //           updateColors(&leftcolor, &rightcolor, &leftnew, &rightnew, mode, isTogether && isLeft, isFirst,fontsize);
  //       }
  //       else
  //       {
  //         if (isLeft)
  //         {
  //           leftcolor.r = rightcolor.r;
  //           leftcolor.g = rightcolor.g;
  //           leftcolor.b = rightcolor.b;
  //         }
  //               else
  //               {
  //                 rightcolor.r = leftcolor.r;
  //                 rightcolor.g = leftcolor.g;
  //                 rightcolor.b = leftcolor.b;
  //               }
  //       }
  //       break;
  //       case 4:
  //         if (mode == 2)
  //         {
  //           changed = true;
  //           if (currentColumn == 2)
  //           {
  //             changetime += 500000;
  //             if (changetime == 10500000) changetime = 500000;
  //           }
  //           else
  //           {
  //             changetime -= 500000;
  //             if (changetime == 0) changetime = 10000000;
  //           }
  //           redrawSetupLine(mode, changetime/100000, selectedsetup, fontsize);
  //         }
  //         else
  //         {
  //           selectedsetup = currentColumn;
  //           redrawSetupLine(mode, 0, selectedsetup, fontsize);
  //           if (selectedsetup == 1) redrawPrefLine(mode, getTextForSetup(selectedsetup), blinktime/100000, fontsize);
  //           else if (selectedsetup == 2) redrawPrefLine(mode, getTextForSetup(selectedsetup), fadetime/100000, fontsize);
  //           else redrawPrefLine(mode, getTextForSetup(selectedsetup), shift/100000, fontsize);
  //         }
  //       break;
  //       case 5:
  //       changed = true;
  //       if (selectedsetup == 1)
  //       {
  //         if (currentColumn == 2)
  //           {
  //             blinktime += 100000;
  //             if (blinktime == 5100000) blinktime = 100000;
  //           }
  //           else
  //           {
  //             blinktime -= 100000;
  //             if (blinktime == 0) blinktime = 5000000;
  //           }
  //           redrawPrefLine(mode, getTextForSetup(selectedsetup), blinktime/100000, fontsize);
  //       }
  //       else if (selectedsetup == 2)
  //       {
  //         if (currentColumn == 2)
  //           {
  //             fadetime += 100000;
  //             if (fadetime == 5100000) fadetime = 100000;
  //           }
  //           else
  //           {
  //             fadetime -= 100000;
  //             if (fadetime == 0) fadetime = 5000000;
  //           }
  //           redrawPrefLine(mode, getTextForSetup(selectedsetup), fadetime/100000, fontsize);
  //       }
  //       else
  //       {
  //         if (currentColumn == 2)
  //           {
  //             shift += 100000;
  //             if (shift == 5100000) shift = 0;
  //           }
  //           else
  //           {
  //             shift -= 100000;
  //             if (shift < 0) shift = 5000000;
  //           }
  //           redrawPrefLine(mode, getTextForSetup(selectedsetup), shift/100000, fontsize);
  //       }
  //       break;
  //       case 6:
  //         fontsize = fontsize == 2? 1 : 2;
  //         if (mode == 2) redrawAll(isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isTogether && isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), changetime/100000, selectedsetup);
  //         else if (mode == 3)
  //         {
  //           if (selectedsetup == 1) redrawAll(isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode,  isTogether && isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), blinktime/100000, selectedsetup);
  //           else if (selectedsetup == 2) redrawAll(isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isTogether && isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), fadetime/100000, selectedsetup);
  //           else redrawAll(isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isTogether && isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), shift/100000, selectedsetup);
  //         }
  //         else redrawAll(isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isTogether && isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), 0, selectedsetup);
  //       break;
  //     }
  //     focus(currentColumn, currentRow, mode);
  //     clockCounter = clock();
  //   }   

  //   for (int x = 0; x < 320; ++x) {
	// 	  for (int y = 0; y < 480; ++y) {
	// 		  parlcd_write_data(parlcd_mem_base, lcdPixels[y][x]);
	// 	  }
  //   }
  //   if (mode == 1)
  //   {
  //     *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&leftcolor);
  //     *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether ? rgbtohex(&leftcolor) : rgbtohex(&rightcolor);
  //   }
  //   else if (mode == 2)
  //   {
  //     if (changed) 
  //     {
  //       before = clock();
  //       *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&leftcolor);
  //       *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? rgbtohex(&leftcolor) : rgbtohex(&rightcolor);
  //       gradleft.r = leftcolor.r;
  //       gradleft.g = leftcolor.g;
  //       gradleft.b = leftcolor.b;
  //       gradleftnew.r = leftnew.r;
  //       gradleftnew.g = leftnew.g;
  //       gradleftnew.b = leftnew.b;
  //       gradright.r = rightcolor.r;
  //       gradright.g = rightcolor.g;
  //       gradright.b = rightcolor.b;
  //       gradrightnew.r = rightnew.r;
  //       gradrightnew.g = rightnew.g;
  //       gradrightnew.b = rightnew.b;
  //       changed = false;
  //     }
  //     else
  //     {
  //       difference = clock() - before;
  //       if (difference >= changetime)
  //       {
  //         before = clock();
  //         char tempr = gradleft.r, tempg = gradleft.g, tempb = gradleft.b;
  //         gradleft.r = gradleftnew.r;
  //         gradleft.g = gradleftnew.g;
  //         gradleft.b = gradleftnew.b;
  //         gradleftnew.r = tempr;
  //         gradleftnew.g = tempg;
  //         gradleftnew.b = tempb;
  //         tempr = gradright.r, tempg = gradright.g, tempb = gradright.b;
  //         gradright.r = gradrightnew.r;
  //         gradright.g = gradrightnew.g;
  //         gradright.b = gradrightnew.b;
  //         gradrightnew.r = tempr;
  //         gradrightnew.g = tempg;
  //         gradrightnew.b = tempb;
  //       }
  //       else
  //       {
  //         t.r = gradleft.r + ((double)difference)/changetime*((int)gradleftnew.r - (int)gradleft.r);
  //         t.g = gradleft.g + ((double)difference)/changetime*((int)gradleftnew.g - (int)gradleft.g);
  //         t.b = gradleft.b + ((double)difference)/changetime*((int)gradleftnew.b - (int)gradleft.b);
  //         *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&t);
  //         if (!isTogether)
  //         {
  //           t.r = gradright.r + ((double)difference)/changetime*((int)gradrightnew.r - (int)gradright.r);
  //           t.g = gradright.g + ((double)difference)/changetime*((int)gradrightnew.g - (int)gradright.g);
  //           t.b = gradright.b + ((double)difference)/changetime*((int)gradrightnew.b - (int)gradright.b);
  //         }
  //         *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = rgbtohex(&t);
  //       }
  //     }
  //   }
  //   else
  //   {
  //     if (changed) 
  //     {
  //       before = clock();
  //       beforer = clock() - shift;
  //       lefton = true;
  //       // righton = (shift == 0);
  //       // *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&leftcolor);
  //       // *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = righton ? (isTogether? rgbtohex(&leftcolor) : rgbtohex(&rightcolor)) : 0x0000;
  //       righton = true;
  //       *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&leftcolor);
  //       *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? rgbtohex(&leftcolor) : rgbtohex(&rightcolor);
  //       changed = false;
  //     }
  //     else
  //     {
  //       difference = clock() - before;
  //       differencer = clock() - beforer;
  //       if (lefton)
  //       {
  //         if (difference >= blinktime)
  //         {
  //           *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = 0x0;
  //           if (shift == 0) 
  //           {
  //             *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = 0x0;
  //             righton = false;
  //           }
  //           lefton = false;
  //           before = clock();
  //         }
  //       }
  //       else
  //       {
  //         if (difference >= fadetime)
  //         {
  //           *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&leftcolor);
  //           if (shift == 0) 
  //           {
  //             *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? rgbtohex(&leftcolor) : rgbtohex(&rightcolor);
  //             righton = true;
  //           }
  //           lefton = true;
  //           before = clock();
  //         }
  //       }
  //       if (shift != 0)
  //       {
  //           if (righton)
  //           {
  //             if (differencer >= blinktime)
  //             {
  //               *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = 0x0;
  //               righton = false;
  //               beforer = clock();
  //             }
  //           }
  //           else
  //           {
  //             if (differencer >= fadetime)
  //             {
  //               *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? rgbtohex(&leftcolor) : rgbtohex(&rightcolor);
  //               righton = true;
  //               beforer = clock();
  //             }
  //           }
  //       }
  //     }
  //   }
  // }
  
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
  ////SEND LIST WITH NUMBERS
  ////copy packet number to end of tempbuf
  sprintf(red, "%d", 180);
  sprintf(green, "%d", 12);
  sprintf(blue, "%d", 175);
  printf(blue);
  //memset(buffer_rx, 0, 9);
  strcat(buffer_rx, red);
  strcat(buffer_rx, green);
  strcat(buffer_rx, blue);
  
  // memcpy(buffer_rx, red, 3); 
  // memcpy(buffer_rx+3, green, 3); 
  // memcpy(buffer_rx+4, blue, 3); 


  sendto(sockfd, buffer_rx, sizeof(buffer_rx), 0, (struct sockaddr*)& addrDest, sizeof(addrDest));
  printf("SENT\n");
  printf("%d\n", sockfd);

  //clear(buffer_rx);
  //recvfrom(sockfd, buffer_tx, 9, 0, (struct sockaddr*)&from, &fromlen);
 // printf("received");
  //printf(buffer_tx);

  // sendto(sockfd, buffer_rx, 1024, 0, (struct sockaddr*)& addrDest, sizeof(addrDest));
  // printf("SENT\n");

  // sendto(sockfd, buffer_rx, 1024, 0, (struct sockaddr*)& addrDest, sizeof(addrDest));
  // printf("SENT\n");

  // sendto(sockfd, buffer_rx, 1024, 0, (struct sockaddr*)& addrDest, sizeof(addrDest));
  // printf("SENT\n");

  // sendto(sockfd, buffer_rx, 1024, 0, (struct sockaddr*)& addrDest, sizeof(addrDest));
  printf("\nMY JOB DONE");
  close(sockfd);


  return 0;
}
#endif _NETWORK_H_