#define _POSIX_C_SOURCE 200112L
#include "screencontrol.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
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
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h> 
uint16_t lcdPixels[480][320];

int toRGB565(char red, char green, char blue)
{
    uint16_t b = (blue >> 3) & 0x1f;
    uint16_t g = ((green >> 2) & 0x3f) <<5;
    uint16_t r = ((red >> 3) & 0x1f) << 11;

    return (uint16_t) (r | g | b);
    
}

void blockColorChange(int blockX, int blockY, int blockXrange, int blockYrange,  int color){
    for(int x = blockX; x < blockX + blockXrange; ++x){
        for(int y = blockY; y < blockY + blockYrange; ++y){
            lcdPixels[x][y] = color;
        }
    }
}

void background(int blockX, int blockY, int color, int blockXsize, int delta){
  for(int x = blockX + delta; x < blockX + blockXsize - delta; ++x){
        for(int y = blockY + delta; y < blockY + 53 - delta; ++y){
            lcdPixels[x][y] = color;
        }
    }

}

void chosenBorder(int blockX, int blockY, int color, bool isDoubled, int delta){
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

void updateColumn(State* state, int* currentRow, int* currentColumn)
{
  switch (state->mode)
      {
      case 1:
        if (*currentRow == 6) {
          *currentColumn = *currentColumn == 3 ? 2 : 3;
          break;
          }
        if (*currentRow == 3) break;
        *currentColumn = *currentColumn == 3 ? 1 : *currentColumn+1;
        break;
      case 2:
        if (*currentRow == 6) {
          *currentColumn = *currentColumn == 3 ? 2 : 3;
          break;
          }
        if (*currentRow == 3 && state->isTogether) 
        {
          *currentColumn = *currentColumn == 1 ? 2 : 1;
          break;
        }
        if (*currentRow == 4)
        {
          *currentColumn = *currentColumn == 3 ? 2 : 3;
          break;
        }
        *currentColumn = *currentColumn == 3 ? 1 : *currentColumn+1;
        break;
      case 3:
        if (*currentRow == 6) {
          *currentColumn = *currentColumn == 3 ? 2 : 3;
          break;
          }
        if (*currentRow == 3) break;
        if (*currentRow == 5)
        {
          *currentColumn = *currentColumn == 3 ? 2 : 3;
          break;
        }
        *currentColumn = *currentColumn == 3 ? 1 : *currentColumn+1;
        break;
      }
}

void updateRow(State* state, int* currentRow, int* currentColumn)
{
  switch (state->mode)
  {
  case 1:
    switch (*currentRow)
    {
    case 2:
      if (state->isTogether)
      {
        *currentRow = 6;
        *currentColumn = 3;
      }
      else
      {
        *currentRow = 3;
        *currentColumn = 3;
      }
    break;
    case 3:
      *currentRow = 6;
      *currentColumn = *currentColumn == 1 ? 2 : *currentColumn;
      break;
    case 6:
      *currentRow = 1;
    break;
    default:
      (*currentRow)++;
      break;
    }
    break;
  case 2:
    switch (*currentRow)
    {
    case 2:
      if (*currentColumn == 3 && state->isTogether) *currentColumn = 2;
      (*currentRow)++;
      break;
    case 3:
      if (*currentColumn == 1) *currentColumn = 2;
      (*currentRow)++;
      break;
    case 4:
      *currentRow = 6;
      *currentColumn = *currentColumn == 1 ? 2 : *currentColumn;
    break;
    case 6:
      *currentRow = 1;
    break;
    default:
      (*currentRow)++;
      break;
    }
    break;
  case 3:
    switch (*currentRow)
    {
    case 2:
      if (state->isTogether) *currentRow = 4;
      else
      {
        *currentRow = 3;
        *currentColumn = 3;
      }
      break;
    case 4:
      if (*currentColumn == 1) *currentColumn = 2;
      (*currentRow)++;
      break;
    case 5:
      *currentRow = 6;
      *currentColumn = *currentColumn == 1 ? 2 : *currentColumn;
    break;
    case 6:
      *currentRow = 1;
    break;
    default:
      (*currentRow)++;
      break;
    }
    break;
  }
}

void updateColors(State* state, bool isLeft, bool isFirst, char fontsize)
{
  background(120,106,0xffff, 240,0);
      if (state->mode == 2)
      {
          if (isLeft)
          {
            background(120,106,toRGB565(state->leftcolor.r, state->leftcolor.g, state->leftcolor.b), 120,3);
            background(240,106,toRGB565(state->leftnew.r, state->leftnew.g, state->leftnew.b), 120,3);
          }
          else
          {
            background(120,106,toRGB565(state->rightcolor.r, state->rightcolor.g, state->rightcolor.b), 120,3);
            background(240,106,toRGB565(state->rightnew.r, state->rightnew.g, state->rightnew.b), 120,3);
          }
          fillBlock("Color 1", 120, 106,120,53,fontsize);
          fillBlock("Color 2", 240, 106,120,53,fontsize);
          focusColor(isFirst? 1 : 2, 3);
      }
      else
      {
          if (isLeft) background(120,106,toRGB565(state->leftcolor.r, state->leftcolor.g, state->leftcolor.b), 240,3);
          else background(120,106,toRGB565(state->rightcolor.r, state->rightcolor.g, state->rightcolor.b), 240,3);
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


char* getTextForSetup(int setup)
{
  if (setup == 1) return "Time on:";
  if (setup == 2) return "Time off:";
  return "Shift:";
}

void redrawPrefLine(State* state, int selectedsetup, char fontsize)
{
  int time = (selectedsetup == 1 ? state->blinktime : (selectedsetup == 2 ? state->fadetime : state->shift))/100000;
  switch (state->mode)
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
    fillBlock(getTextForSetup(selectedsetup), 0, 212,120,53,fontsize);
    char str[3];
    sprintf(str, "%d,%d", time/10, time%10); 
    fillBlock(str, 120, 212,120,53,fontsize);
    fillBlock("+0,1", 240, 212,120,53,fontsize);
    fillBlock("-0,1", 360, 212,120,53,fontsize);
    break;
  }
}

void redrawConstants(bool isSender, char fontsize)
{
  background(0,0,0xD6DA, 120,0);
  fillBlock("LED:", 0, 0,120,53,fontsize);
  background(0,53,0xD6DA, 120,0);
  fillBlock("Mode:", 0, 53,120,53,fontsize);
  background(0,106,0xD6DA, 120,0);
  fillBlock("Color:", 0, 106,120,53,fontsize);
  background(0,265,0xD6DA, 240,0);
  background(360,265,0xffff, 120,0);
  fillBlock("Font", 360, 265,120,53,fontsize);
  redrawConnection(isSender, fontsize);
}

void redrawConnection(bool sender, char fontsize)
{
  if (sender)
  {
    background(240,265,0xffff, 120,0);
    fillBlock("Send", 240, 265,120,53,fontsize);
  }
  else
  {
    background(240,265,0xffff, 120,0);
    fillBlock("Self", 240, 265,120,53,fontsize);
  }
}

void redrawAll(State * state, bool isSender,  bool isLeft, bool isFirst, char fontsize, int selectedsetup)
{
  redrawConstants(isSender, fontsize);
  redrawLEDLine(state->isTogether, isLeft,fontsize);
  redrawModeLine(state->mode,fontsize);
  updateColors(state, isLeft, isFirst,fontsize);
  setCopyButton(true,fontsize);
  redrawSetupLine(state->mode, state->changetime/100000, selectedsetup,fontsize);
  redrawPrefLine(state, selectedsetup, fontsize);
}

void drawReceiver()
{
    background(0,0,0xffff, 480,0);
    background(0,53,0xffff, 480,0);
    background(0,106,0xffff, 480,0);
    background(0,159,0xffff, 480,0);
    background(0,212,0xffff, 480,0);
    background(0,265,0xffff, 480,0);
    fillBlock("Press any knob to", 0, 0,480,53,2);
    fillBlock("leave receiver mode", 0, 53,480,53,2);
}

void drawScreen(unsigned char parlcd_mem_base)
{
  for (int x = 0; x < 320; ++x) {
        for (int y = 0; y < 480; ++y) {
          parlcd_write_data(parlcd_mem_base, lcdPixels[y][x]);
        }
      }
}