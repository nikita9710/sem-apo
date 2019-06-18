#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
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
#define LOCAL_PORT 8888
uint16_t lcdPixels[480][320];


typedef struct 
{
  char r;
  char g;
  char b;
} RGB;

typedef struct
{
  int mode; 
  bool isTogether;
  RGB leftcolor, rightcolor, leftnew, rightnew;
  int changetime;
  int blinktime, fadetime, shift;
} State;

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

bool updateDataBool(bool* data, char* buffer_rx, char * offset, const char datasize)
{

    bool changed = false;
    bool tempbool;
    char * temp[datasize];
    memcpy(temp, buffer_rx+*offset, datasize);
    tempbool = (int)strtol(temp, NULL, 16);
    *offset += datasize;
    if (*data != tempbool)
    {
      *data = tempbool;
      printf("%d\n", *data);
      return true;
    }
    return false;
}

bool updateDataInt(int* data, char* buffer_rx, char * offset, const char datasize)
{

    bool changed = false;
    int tempint;
    char * temp[datasize];
    memcpy(temp, buffer_rx+*offset, datasize);
    tempint = (int)strtol(temp, NULL, 16);
    *offset += datasize;
    if (*data != tempint)
    {
      *data = tempint;
      printf("%d\n", *data);
      return true;
    }
    return false;
}

bool updateDataChar(char* color, char* buffer_rx, char * offset, const char datasize)
{

    bool changed = false;
    char tempint;
    char temp[datasize];
    memcpy(temp, buffer_rx+*offset, datasize);
    tempint = (int)strtol(temp, NULL, 16);
    *offset += datasize;
    if (*color != tempint)
    {
      *color = tempint;
      printf("%d\n", *color);
      return true;
    }
    return false;
}

bool updateColor(RGB* color, char* buffer_rx, char * offset)
{
  bool changed = false;
  changed = changed | updateDataChar(&(color->r), buffer_rx, offset, 2);
  changed = changed | updateDataChar(&(color->g), buffer_rx, offset, 2);
  changed = changed | updateDataChar(&(color->b), buffer_rx, offset, 2);
  return changed;
}

bool updateState(State* state, char* buffer_rx)
{
  char offset = 0;
  bool changed = false;
  changed = changed | updateDataBool(&(state->isTogether), buffer_rx, &offset, 1);
  changed = changed | updateDataInt(&(state->mode), buffer_rx, &offset, 1);
  changed = changed | updateColor(&(state->leftcolor), buffer_rx, &offset);
  changed = changed | updateColor(&(state->rightcolor), buffer_rx, &offset);
  changed = changed | updateColor(&(state->leftnew), buffer_rx, &offset);
  changed = changed | updateColor(&(state->rightnew), buffer_rx, &offset);
  changed = changed | updateDataInt(&(state->changetime), buffer_rx, &offset, 6);
  changed = changed | updateDataInt(&(state->blinktime), buffer_rx, &offset, 6);
  changed = changed | updateDataInt(&(state->fadetime), buffer_rx, &offset, 6);
  changed = changed | updateDataInt(&(state->shift), buffer_rx, &offset, 6);
  return changed;
}


void increasecolor(RGB* color, char dr, char dg, char db)
{
  color->r += dr;
  color->g += dg;
  color->b += db;
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

void parepareData(State* state, char* buffer_rx)
{
  char strTogether[1], strMode[1], 
    strredLeft[2],strgreenLeft[2], strblueLeft[2],
    strredRight[2], strgreenRight[2], strblueRight[2],
    strredLeftnew[2],strgreenLeftnew[2], strblueLeftnew[2], 
    strredRightnew[2], strgreenRightnew[2], strblueRightnew[2],
    strfadetime[6], strblinkshift[6], strblinktime[6], strchangetime[6];
  memset(buffer_rx, 0, 60);
  sprintf(strTogether, "%d", state->isTogether);
  strcat(buffer_rx, strTogether);
  sprintf(strMode, "%d", state->mode);
  strcat(buffer_rx, strMode);
  sprintf(strredLeft, "%02x", state->leftcolor.r);
  strcat(buffer_rx, strredLeft);
  sprintf(strgreenLeft, "%02x", state->leftcolor.g);
  strcat(buffer_rx, strgreenLeft);
  sprintf(strblueLeft, "%02x", state->leftcolor.b);
  strcat(buffer_rx, strblueLeft);
  sprintf(strredRight, "%02x", state->rightcolor.r);
  strcat(buffer_rx, strredRight);
  sprintf(strgreenRight, "%02x", state->rightcolor.g);
  strcat(buffer_rx, strgreenRight);
  sprintf(strblueRight, "%02x", state->rightcolor.b);
  strcat(buffer_rx, strblueRight);
  sprintf(strredLeftnew, "%02x", state->leftnew.r);
  strcat(buffer_rx, strredLeftnew);
  sprintf(strgreenLeftnew, "%02x", state->leftnew.g);
  strcat(buffer_rx, strgreenLeftnew);
  sprintf(strblueLeftnew, "%02x", state->leftnew.b);
  strcat(buffer_rx, strblueLeftnew);
  sprintf(strredRightnew, "%02x", state->rightnew.r);
  strcat(buffer_rx, strredRightnew);
  sprintf(strgreenRightnew, "%02x", state->rightnew.g);
  strcat(buffer_rx, strgreenRightnew);
  sprintf(strblueRightnew, "%02x", state->rightnew.b);
  strcat(buffer_rx, strblueRightnew);
  sprintf(strchangetime, "%06x", state->changetime);
  strcat(buffer_rx, strchangetime);
  sprintf(strblinktime, "%06x", state->blinktime);
  strcat(buffer_rx, strblinktime);
  sprintf(strfadetime, "%06x", state->fadetime);
  strcat(buffer_rx, strfadetime);
  sprintf(strblinkshift, "%06x", state->shift);
  strcat(buffer_rx, strblinkshift);
  printf("\nInfo Sent");
}

void resetState(State* state)
{
  state->mode = 1;
  state->isTogether = false;
  state->leftcolor.r = 243;
  state->leftcolor.g = 31;
  state->leftcolor.b = 83;
  state->rightcolor.r = 40;
  state->rightcolor.g = 240;
  state->rightcolor.b = 250;
  state->leftnew.r = 192;
  state->leftnew.g = 56;
  state->leftnew.b = 222;
  state->rightnew.r = 10;
  state->rightnew.g =130;
  state->rightnew.b = 200;
  state->changetime = 1000000;
  state->blinktime = 1000000;
  state->fadetime = 1000000;
  state->shift = 500000;
}

int main(int argc, char *argv[])
{ 
  //for all
  char buffer_rx[60];
  int sockfd = -1;
  struct sockaddr_in bindaddr;  
  struct sockaddr_in braddr;
  fd_set readfds, masterfds;
  bool receiver = false, sender = false;

  unsigned char *mem_base;
  mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
  if (mem_base == NULL) exit(1);

  uint32_t rgb_knobs_value;
  int rk, gk, bk,rb, gb, bb;
  clock_t before = 0;
  clock_t beforer = 0;
  clock_t clockCounter = clock();
  clock_t difference = 0;  
  clock_t differencer = 0; 
  State state = {1, false, {243, 31, 83}, {40, 240, 250},{192,56,222},{10,130,200}, 1000000, 1000000, 1000000, 500000};
  rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
  int rprev = (rgb_knobs_value>>16) & 0xFF, gprev = (rgb_knobs_value>>8)  & 0xFF, bprev = rgb_knobs_value      & 0xFF;
  RGB t = {0,0,0}, 
      gradleft = {0,0,0}, gradright = {0,0,0}, 
      gradleftnew = {0,0,0}, gradrightnew = {0,0,0};
  bool lefton = true, righton = true, changed = true, isLeft = true, isFirst = true;
  int selectedsetup = 1;
  int currentColumn = 1, prevColumn = 1;
  int currentRow = 1, prevRow = 1;
  int fontsize = 1;
  unsigned char *parlcd_mem_base;
  parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
  if (parlcd_mem_base == NULL)  exit(1);
  parlcd_hx8357_init(parlcd_mem_base);
  *(volatile uint16_t*)(parlcd_mem_base + PARLCD_REG_CMD_o) = 0x2c;

  redrawAll(&state, sender, isLeft, isFirst, fontsize, selectedsetup);
  focus(currentColumn, currentRow, state.mode);

  while (1)
  {
    rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    bk =  rgb_knobs_value      & 0xFF; // blue knob position
    gk = (rgb_knobs_value>>8)  & 0xFF; // green knob position
    rk = (rgb_knobs_value>>16) & 0xFF; // red knob position
    bb = (rgb_knobs_value>>24) & 1;    // blue button
    gb = (rgb_knobs_value>>25) & 1;    // green button
    rb = (rgb_knobs_value>>26) & 1;    // red buttom
    if (receiver)
    {
      if ((bb || gb || rb)  && (clock()-clockCounter>=300000)) 
      {
        close(sockfd);
        sockfd = socket(AF_INET,SOCK_DGRAM,0);
        int broadcast = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
            sizeof broadcast) == -1) {
            perror("setsockopt (SO_BROADCAST)");
            exit(1);
        }
        receiver = false;
        sender = true;
        resetState(&state);
        memset(&braddr, 0, sizeof(braddr));
        braddr.sin_family = AF_INET;
        braddr.sin_port = htons(LOCAL_PORT);
        braddr.sin_addr.s_addr = INADDR_BROADCAST;
        redrawAll(&state, sender, isLeft, isFirst, fontsize, selectedsetup);
        focus(currentColumn, currentRow, state.mode);
        clockCounter = clock();
        continue;
      }

      FD_ZERO(&masterfds);
      FD_SET(sockfd, &masterfds);
      memcpy(&readfds, &masterfds, sizeof(fd_set));
      fcntl(sockfd, F_SETFL, O_NONBLOCK);

      recvfrom(sockfd, buffer_rx, 60, 0, (struct sockaddr*)&braddr, &braddr);

      changed = updateState(&state, buffer_rx);
    }
    else
    {
      if (rprev != rk || gprev != gk || bprev != bk)
      {
        int dr = rk-rprev,db = bk-bprev, dg = gk-gprev;
        rprev = rk;
        gprev = gk;
        bprev = bk;
        if (isLeft)
        {
          if (state.mode == 2 && !isFirst) increasecolor(&state.leftnew, dr, db, db);
          else increasecolor(&state.leftcolor, dr, dg, db);
        }
        else
        {
          if (state.mode == 2 && !isFirst)increasecolor(&state.rightnew, dr, dg, db);
          else increasecolor(&state.rightcolor, dr, dg, db);
        }
        changed = true;
        updateColors(&state, isLeft, isFirst,fontsize);
        focus(currentColumn, currentRow, state.mode);
      }


      if(rb && (clock()-clockCounter>=180000)){
        prevColumn = currentColumn;
        prevRow = currentRow;
        updateColumn(&state, &currentRow, &currentColumn);
        setfocus(currentColumn, currentRow, prevColumn, prevRow, state.mode);
        clockCounter = clock();
      }


      if(gb && (clock()-clockCounter>=180000)){
        prevColumn = currentColumn;
        prevRow = currentRow;
        updateRow(&state, &currentRow, &currentColumn);
        setfocus(currentColumn, currentRow, prevColumn, prevRow, state.mode);
        clockCounter = clock();
      }

      if(bb && (clock()-clockCounter>=300000)){
        switch (currentRow)
        {
        case 1:
          changed = true;
          switch (currentColumn)
          {
          case 1:
            isLeft = true;
            state.isTogether = false;
            break;
          case 2:
            isLeft = false;
            state.isTogether = false;
            break;
          case 3:
            isLeft = true;
            state.isTogether = true;
            break;
          }
          redrawLEDLine(state.isTogether, isLeft,fontsize);
          updateColors(&state, isLeft, isFirst,fontsize);
          setCopyButton(!state.isTogether,fontsize);
          focus(currentColumn, currentRow, state.mode);
          break;
        case 2:
          changed = true;
          state.mode = currentColumn;
          redrawSetupLine(state.mode,state.changetime/100000, selectedsetup,fontsize);
          redrawPrefLine(&state, selectedsetup,fontsize);
          redrawModeLine(state.mode,fontsize);
          updateColors(&state, isLeft, isFirst,fontsize);
          break;
        case 3:
          changed = true;
          if (state.mode == 2)
          {
              switch (currentColumn)
              {
                case 1:
                  isFirst = true;
                break;
                case 2:
                  isFirst = false;
                break;
                case 3:
                  if (isLeft)
                  {
                    state.leftcolor.r = state.rightcolor.r;
                    state.leftcolor.g = state.rightcolor.g;
                    state.leftcolor.b = state.rightcolor.b;
                    state.leftnew.r = state.rightnew.r;
                    state.leftnew.g = state.rightnew.g;
                    state.leftnew.b = state.rightnew.b;
                  }
                  else
                  {
                    state.rightcolor.r = state.leftcolor.r;
                    state.rightcolor.g = state.leftcolor.g;
                    state.rightcolor.b = state.leftcolor.b;
                    state.rightnew.r = state.leftnew.r;
                    state.rightnew.g = state.leftnew.g;
                    state.rightnew.b = state.leftnew.b;
                  }
                  updateColors(&state, isLeft, isFirst,fontsize);
                  break;
              }
          }
          else
          {
            if (isLeft)
            {
              state.leftcolor.r = state.rightcolor.r;
              state.leftcolor.g = state.rightcolor.g;
              state.leftcolor.b = state.rightcolor.b;
            }
            else
            {
              state.rightcolor.r = state.leftcolor.r;
              state.rightcolor.g = state.leftcolor.g;
              state.rightcolor.b = state.leftcolor.b;
            }
            updateColors(&state, isLeft, isFirst,fontsize);
          }
          break;
          case 4:
            if (state.mode == 2)
            {
              changed = true;
              if (currentColumn == 2)
              {
                state.changetime += 500000;
                if (state.changetime == 10500000) state.changetime = 500000;
              }
              else
              {
                state.changetime -= 500000;
                if (state.changetime == 0) state.changetime = 10000000;
              }
              redrawSetupLine(state.mode, state.changetime/100000, selectedsetup, fontsize);
            }
            else
            {
              selectedsetup = currentColumn;
              redrawSetupLine(state.mode, 0, selectedsetup, fontsize);
              redrawPrefLine(&state, selectedsetup,fontsize);
            }
            break;
          case 5:
            changed = true;
            if (selectedsetup == 1)
            {
              if (currentColumn == 2)
                {
                  state.blinktime += 100000;
                  if (state.blinktime == 5100000) state.blinktime = 100000;
                }
                else
                {
                  state.blinktime -= 100000;
                  if (state.blinktime == 0) state.blinktime = 5000000;
                }
            }
            else if (selectedsetup == 2)
            {
              if (currentColumn == 2)
                {
                  state.fadetime += 100000;
                  if (state.fadetime == 5100000) state.fadetime = 100000;
                }
                else
                {
                  state.fadetime -= 100000;
                  if (state.fadetime == 0) state.fadetime = 5000000;
                }
            }
            else
            {
              if (currentColumn == 2)
                {
                  state.shift += 100000;
                  if (state.shift == 5100000) state.shift = 0;
                }
                else
                {
                  state.shift -= 100000;
                  if (state.shift < 0) state.shift = 5000000;
                }
            }
            redrawPrefLine(&state, selectedsetup,fontsize);
            break;
          case 6:
            if (currentColumn == 3)
            {
              fontsize = fontsize == 2? 1 : 2;
              redrawAll(&state, sender, isLeft, isFirst, fontsize, selectedsetup);
            }
            else
            {
              if (sender)
              {
                close(sockfd);
                receiver = false;
                sender = false;
                redrawConnection(sender, fontsize);
              }
              else
              {
                if (sockfd != -1) close(sockfd);
                sockfd = socket(AF_INET,SOCK_DGRAM,0);
                int broadcast = 1;
                if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
                    sizeof broadcast) == -1) {
                    perror("setsockopt (SO_BROADCAST)");
                    exit(1);
                }
                receiver = true;
                sender = false;
                memset(&bindaddr, 0, sizeof(bindaddr));
                bindaddr.sin_family = AF_INET;
                bindaddr.sin_port = htons(LOCAL_PORT);
                bindaddr.sin_addr.s_addr = INADDR_ANY;
                if (bind(sockfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) == -1) {
                  perror("bind");
                  exit(1);
                }
                drawReceiver();
              }
            }
            break;
        }
        if (!receiver) focus(currentColumn, currentRow, state.mode);
        clockCounter = clock();
      }   

      for (int x = 0; x < 320; ++x) {
        for (int y = 0; y < 480; ++y) {
          parlcd_write_data(parlcd_mem_base, lcdPixels[y][x]);
        }
      }
    }

    if (changed && sender)
    {
      parepareData(&state, buffer_rx);
      sendto(sockfd, buffer_rx, sizeof(buffer_rx), 0, (struct sockaddr*)&braddr, sizeof(braddr));
    }

    if (state.mode == 1)
    {
      *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&state.leftcolor);
      *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = state.isTogether ? rgbtohex(&state.leftcolor) : rgbtohex(&state.rightcolor);
      changed = false;
    }
    else if (state.mode == 2)
    {
      if (changed) 
      {
        before = clock();
        *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&state.leftcolor);
        *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = state.isTogether? rgbtohex(&state.leftcolor) : rgbtohex(&state.rightcolor);
        gradleft.r = state.leftcolor.r;
        gradleft.g = state.leftcolor.g;
        gradleft.b = state.leftcolor.b;
        gradleftnew.r = state.leftnew.r;
        gradleftnew.g = state.leftnew.g;
        gradleftnew.b = state.leftnew.b;
        gradright.r = state.rightcolor.r;
        gradright.g = state.rightcolor.g;
        gradright.b = state.rightcolor.b;
        gradrightnew.r = state.rightnew.r;
        gradrightnew.g = state.rightnew.g;
        gradrightnew.b = state.rightnew.b;
        changed = false;
      }
      else
      {
        difference = clock() - before;
        if (difference >= state.changetime)
        {
          before = clock();
          char tempr = gradleft.r, tempg = gradleft.g, tempb = gradleft.b;
          gradleft.r = gradleftnew.r;
          gradleft.g = gradleftnew.g;
          gradleft.b = gradleftnew.b;
          gradleftnew.r = tempr;
          gradleftnew.g = tempg;
          gradleftnew.b = tempb;
          tempr = gradright.r, tempg = gradright.g, tempb = gradright.b;
          gradright.r = gradrightnew.r;
          gradright.g = gradrightnew.g;
          gradright.b = gradrightnew.b;
          gradrightnew.r = tempr;
          gradrightnew.g = tempg;
          gradrightnew.b = tempb;
        }
        else
        {
          t.r = gradleft.r + ((double)difference)/state.changetime*((int)gradleftnew.r - (int)gradleft.r);
          t.g = gradleft.g + ((double)difference)/state.changetime*((int)gradleftnew.g - (int)gradleft.g);
          t.b = gradleft.b + ((double)difference)/state.changetime*((int)gradleftnew.b - (int)gradleft.b);
          *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&t);
          if (!state.isTogether)
          {
            t.r = gradright.r + ((double)difference)/state.changetime*((int)gradrightnew.r - (int)gradright.r);
            t.g = gradright.g + ((double)difference)/state.changetime*((int)gradrightnew.g - (int)gradright.g);
            t.b = gradright.b + ((double)difference)/state.changetime*((int)gradrightnew.b - (int)gradright.b);
          }
          *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = rgbtohex(&t);
        }
      }
    }
    else
    {
      if (changed) 
      {
        before = clock();
        beforer = clock() - state.shift;
        lefton = true;
        righton = true;
        *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&state.leftcolor);
        *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = state.isTogether? rgbtohex(&state.leftcolor) : rgbtohex(&state.rightcolor);
        changed = false;
      }
      else
      {
        difference = clock() - before;
        differencer = clock() - beforer;
        if (lefton)
        {
          if (difference >= state.blinktime)
          {
            *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = 0x0;
            if (state.shift == 0) 
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
          if (difference >= state.fadetime)
          {
            *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&state.leftcolor);
            if (state.shift == 0) 
            {
              *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = state.isTogether? rgbtohex(&state.leftcolor) : rgbtohex(&state.rightcolor);
              righton = true;
            }
            lefton = true;
            before = clock();
          }
        }
        if (state.shift != 0)
        {
            if (righton)
            {
              if (differencer >= state.blinktime)
              {
                *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = 0x0;
                righton = false;
                beforer = clock();
              }
            }
            else
            {
              if (differencer >= state.fadetime)
              {
                *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = state.isTogether? rgbtohex(&state.leftcolor) : rgbtohex(&state.rightcolor);
                righton = true;
                beforer = clock();
              }
            }
        }
      }
    }
  }
  
  return 0;
}
