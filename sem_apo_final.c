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

void redrawConstants(bool receiver, bool sender, char fontsize)
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
  redrawConnection(receiver, sender, fontsize);
}

void redrawConnection(bool receiver, bool sender, char fontsize)
{
  if (receiver)
  {
    background(240,265,0xffff, 120,0);
    fillBlock("Receive", 240, 265,120,53,fontsize);
  }
  else if (sender)
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

void redrawAll(bool receiver, bool sender, bool isTogether, RGB* leftcolor, RGB* rightcolor,  RGB* leftnew, RGB* rightnew, int mode, bool isLeft, bool isFirst, char fontsize, char* text, int time, int selectedsetup)
{
  redrawConstants(receiver, sender, fontsize);
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

int main(int argc, char *argv[])
{ 
  //for all
  
  int sockfd = -1;
  struct sockaddr_in bindaddr;  
  struct sockaddr_in braddr;
  bool receiver = false, sender = false;
  

   


  char  buffer_rx[60], strTogether[1], strMode[1], redLeft[2],greenLeft[2], blueLeft[2],redRight[2], greenRight[2], blueRight[2],
      redLeftnew[2],greenLeftnew[2], blueLeftnew[2], redRightnew[2], greenRightnew[2], blueRightnew[2],
     intfadetime[6], intblinkshift[6], intblinktime[6], intchangetime[6];

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
  bool isTogether = false;
  RGB leftcolor = {243, 31, 83}, rightcolor = {40, 240, 250}, 
      leftnew = {192,56,222}, rightnew = {10,130,200};
  int mode = 1; //1 - still, 2 - gradient, 3 - blinkink
  int changetime = 1000000;
  int blinktime = 1000000, fadetime = 1000000, shift = 500000;

  char tempcolor;
  bool temptogether;
  int tempint;
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


  if (mode == 2) redrawAll(receiver, sender, isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isTogether, isFirst,fontsize, getTextForSetup(selectedsetup), changetime/100000, selectedsetup);
  else if (mode == 3)
  {
    if (selectedsetup == 1) redrawAll(receiver, sender, isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), blinktime/100000, selectedsetup);
    else if (selectedsetup == 2) redrawAll(receiver, sender, isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), fadetime/100000, selectedsetup);
    else redrawAll(receiver, sender, isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), shift/100000, selectedsetup);
  }
  else redrawAll(receiver, sender, isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), 0, selectedsetup);
  focus(currentColumn, currentRow, mode);


  fd_set readfds, masterfds;

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
        memset(&braddr, 0, sizeof(braddr));
        braddr.sin_family = AF_INET;
        braddr.sin_port = htons(LOCAL_PORT);
        braddr.sin_addr.s_addr = INADDR_BROADCAST;
        if (mode == 2) redrawAll(receiver, sender, isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), changetime/100000, selectedsetup);
        else if (mode == 3)
        {
          if (selectedsetup == 1) redrawAll(receiver, sender, isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode,  isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), blinktime/100000, selectedsetup);
          else if (selectedsetup == 2) redrawAll(receiver, sender, isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), fadetime/100000, selectedsetup);
          else redrawAll(receiver, sender, isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), shift/100000, selectedsetup);
        }
        else redrawAll(receiver, sender, isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), 0, selectedsetup);
        focus(currentColumn, currentRow, mode);
        clockCounter = clock();
        continue;
      }

      FD_ZERO(&masterfds);
      FD_SET(sockfd, &masterfds);
      memcpy(&readfds, &masterfds, sizeof(fd_set));
      fcntl(sockfd, F_SETFL, O_NONBLOCK);

      recvfrom(sockfd, buffer_rx, 60, 0, (struct sockaddr*)&braddr, &braddr);

      memcpy(strTogether, buffer_rx, 1);
      temptogether = (int)strtol(strTogether, NULL, 16);
      if (temptogether != isTogether)
      {
        isTogether = temptogether;
        changed = true;
        printf("isTogether %d\n", isTogether);
      }
      

      memcpy(strMode, buffer_rx+1, 1);
      tempint = (int)strtol(strMode, NULL, 16);
      if (tempint != mode)
      {
        mode = tempint;
        changed = true; 
      printf("Mode is %d\n", mode);   
      }

      memcpy(redLeft, buffer_rx+2, 2);
      tempcolor = (int)strtol(redLeft, NULL, 16);
      if (leftcolor.r != tempcolor)
      {
        leftcolor.r = tempcolor;
        changed = true;
      printf("leftcolor.r is %d\n", leftcolor.r);
      }

      memcpy(greenLeft, buffer_rx+4, 2);
      tempcolor = (int)strtol(greenLeft, NULL, 16);
      if (leftcolor.g != tempcolor)
      {
        leftcolor.g = tempcolor;
        changed = true;
      printf("leftcolor.g is %d\n", leftcolor.g);
      }

      memcpy(blueLeft, buffer_rx+6, 2);
      tempcolor = (int)strtol(blueLeft, NULL, 16);
      if (leftcolor.b != tempcolor)
      {
        leftcolor.b = tempcolor;
        changed = true;
      printf("leftcolor.b is %d\n", leftcolor.b);
      }

      memcpy(redRight, buffer_rx+8, 2);
      tempcolor = (int)strtol(redRight, NULL, 16);
      if (rightcolor.r != tempcolor)
      {
        rightcolor.r = tempcolor;
        changed = true;
      printf("rightcolor.r is %d\n", rightcolor.r);
      }

      memcpy(greenRight, buffer_rx+10, 2);
      tempcolor = (int)strtol(greenRight, NULL, 16);
      if (rightcolor.g != tempcolor)
      {
        rightcolor.g = tempcolor;
        changed = true;
      printf("rightcolor.g is %d\n", rightcolor.g);
      }

      memcpy(blueRight, buffer_rx+12, 2);
      tempcolor = (int)strtol(blueRight, NULL, 16);
      if (rightcolor.b != tempcolor)
      {
        rightcolor.b = tempcolor;
        changed = true;
      printf("rightcolor.b is %d\n", rightcolor.b);
      }

      memcpy(redLeftnew, buffer_rx+14, 2);
      tempcolor = (int)strtol(redLeftnew, NULL, 16);
      if (leftnew.r != tempcolor)
      {
        leftnew.r = tempcolor;
        changed = true;
      printf("leftnew.r is %d\n", leftnew.r);
      }

      memcpy(greenLeftnew, buffer_rx+16, 2);
      tempcolor = (int)strtol(greenLeftnew, NULL, 16);
      if (leftnew.g != tempcolor)
      {
        leftnew.g = tempcolor;
        changed = true;
      printf("leftnew.g is %d\n", leftnew.g);
      }

      memcpy(blueLeftnew, buffer_rx+18, 2);
      tempcolor = (int)strtol(blueLeftnew, NULL, 16);
      if (leftnew.b != tempcolor)
      {
        leftnew.b = tempcolor;
        changed = true;
      printf("leftnew.b is %d\n", leftnew.b);
      }

      memcpy(redRightnew, buffer_rx+20, 2);
      tempcolor = (int)strtol(redRightnew, NULL, 16);
      if (rightnew.r != tempcolor)
      {
        rightnew.r = tempcolor;
        changed = true;
      printf("rightnew.r is %d\n", rightnew.r);
      }

      memcpy(greenRightnew, buffer_rx+22, 2);
      tempcolor = (int)strtol(greenRightnew, NULL, 16);
      if (rightnew.g != tempcolor)
      {
        rightnew.g = tempcolor;
        changed = true;
      printf("rightnew.g is %d\n", rightnew.g);
      }
      memcpy(blueRightnew, buffer_rx+24, 2);
      tempcolor = (int)strtol(blueRightnew, NULL, 16);
      if (rightnew.b != tempcolor)
      {
        rightnew.b = tempcolor;
        changed = true;
      printf("rightnew.b is %d\n", rightnew.b);
      }

      memcpy(intchangetime, buffer_rx + 26, 6);
      tempint = (int)strtol(intchangetime, NULL, 16);
      if (tempint != changetime)
      {
        changetime = tempint;
        changed = true;
      printf("change is %d\n", changetime);
      }

      memcpy(intblinktime, buffer_rx +32, 6);
      tempint = (int)strtol(intblinktime, NULL, 16);
      if (tempint != blinktime)
      {
        blinktime = tempint;
        changed = true;
      printf("blink is %d\n", blinktime);
    
      }
      memcpy(intfadetime, buffer_rx+38, 6);
      tempint = (int)strtol(intfadetime, NULL, 16);
      if (tempint != fadetime)
      {
        fadetime = tempint;
        changed = true;
      printf("fade is %d\n", fadetime);
      }
      memcpy(intblinkshift, buffer_rx+44, 6);
      tempint = (int)strtol(intblinkshift, NULL, 16);
      if (tempint != shift)
      {
        shift = tempint;
        changed = true;
      printf("shift is %d\n", shift);
      }  
    }
    else
    {
    if (rprev != rk || gprev != gk || bprev != bk)
    {
      int dr = rk-rprev,db = bk-bprev,dg = gk-gprev;
      rprev = rk;
      gprev = gk;
      bprev = bk;
      if (isLeft)
      {
        if (mode == 2 && !isFirst)
        {
        leftnew.r += dr;
        leftnew.g += dg;
        leftnew.b += db;
        }
        else
        {
        leftcolor.r += dr;
        leftcolor.g += dg;
        leftcolor.b += db;
        }
      }
      else
      {
        if (mode == 2 && !isFirst)
        {
        rightnew.r += dr;
        rightnew.g += dg;
        rightnew.b += db;
        }
        else
        {
        rightcolor.r += dr;
        rightcolor.g += dg;
        rightcolor.b += db;
        }
      }
      changed = true;
      updateColors(&leftcolor, &rightcolor, &leftnew, &rightnew, mode, isLeft, isFirst,fontsize);
      focus(currentColumn, currentRow, mode);
    }


    if(rb && (clock()-clockCounter>=180000)){
      prevColumn = currentColumn;
      prevRow = currentRow;
      switch (mode)
      {
      case 1:
        if (currentRow == 6) {
          currentColumn = currentColumn == 3 ? 2 : 3;
          break;
          }
        if (currentRow == 3) break;
        currentColumn = currentColumn == 3 ? 1 : currentColumn+1;
        break;
      case 2:
        if (currentRow == 6) {
          currentColumn = currentColumn == 3 ? 2 : 3;
          break;
          }
        if (currentRow == 3 && isTogether) 
        {
          currentColumn = currentColumn == 1 ? 2 : 1;
          break;
        }
        if (currentRow == 4)
        {
          currentColumn = currentColumn == 3 ? 2 : 3;
          break;
        }
        currentColumn = currentColumn == 3 ? 1 : currentColumn+1;
        break;
      case 3:
        if (currentRow == 6) {
          currentColumn = currentColumn == 3 ? 2 : 3;
          break;
          }
        if (currentRow == 3) break;
        if (currentRow == 5)
        {
          currentColumn = currentColumn == 3 ? 2 : 3;
          break;
        }
        currentColumn = currentColumn == 3 ? 1 : currentColumn+1;
        break;
      }
      setfocus(currentColumn, currentRow, prevColumn, prevRow, mode);
      clockCounter = clock();
    }


    if(gb && (clock()-clockCounter>=180000)){
      prevColumn = currentColumn;
      prevRow = currentRow;
      switch (mode)
      {
      case 1:
        switch (currentRow)
        {
        case 2:
          if (isTogether)
          {
            currentRow = 6;
            currentColumn = 3;
          }
          else
          {
            currentRow = 3;
            currentColumn = 3;
          }
        break;
        case 3:
          currentRow = 6;
          currentColumn = currentColumn == 1 ? 2 : currentColumn;
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
        case 3:
          if (currentColumn == 1) currentColumn = 2;
          currentRow++;
          break;
        case 4:
          currentRow = 6;
          currentColumn = currentColumn == 1 ? 2 : currentColumn;
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
          if (isTogether) currentRow = 4;
          else
          {
            currentRow = 3;
            currentColumn = 3;
          }
          break;
        case 4:
          if (currentColumn == 1) currentColumn = 2;
          currentRow++;
          break;
        case 5:
          currentRow = 6;
          currentColumn = currentColumn == 1 ? 2 : currentColumn;
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
      setfocus(currentColumn, currentRow, prevColumn, prevRow, mode);
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
          if (!isLeft || isTogether) changed = true;
          isLeft = true;
          isTogether = false;
          break;
        case 2:
          if (isLeft || isTogether) changed = true;
          isLeft = false;
          isTogether = false;
          break;
        case 3:
          if (!isTogether) changed = true;
          isLeft = true;
          isTogether = true;
          break;
        }
        redrawLEDLine(isTogether, isLeft,fontsize);
        updateColors(&leftcolor, &rightcolor, &leftnew, &rightnew, mode, isLeft, isFirst,fontsize);
        setCopyButton(!isTogether,fontsize);
        focus(currentColumn, currentRow, mode);
        break;
      case 2:
        changed = true;
        switch (currentColumn)
        {
        case 1:
          mode = 1;
          redrawSetupLine(1,0, selectedsetup,fontsize);
          redrawPrefLine(1, "", 0,fontsize);
          break;
        case 2:
          mode = 2;
          redrawSetupLine(2,changetime/100000, selectedsetup,fontsize);
          redrawPrefLine(2, "", 0,fontsize);
          break;
        case 3:
          mode = 3;
          redrawSetupLine(3,0, selectedsetup,fontsize);
          redrawPrefLine(3, "Time on:", blinktime/100000,fontsize);
          break;
        }
        redrawModeLine(mode,fontsize);
        updateColors(&leftcolor, &rightcolor,  &leftnew, &rightnew, mode, isLeft, isFirst,fontsize);
        break;
      case 3:
        changed = true;
        if (mode == 2)
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
                  leftcolor.r = rightcolor.r;
                  leftcolor.g = rightcolor.g;
                  leftcolor.b = rightcolor.b;
                  leftnew.r = rightnew.r;
                  leftnew.g = rightnew.g;
                  leftnew.b = rightnew.b;
                }
                else
                {
                  rightcolor.r = leftcolor.r;
                  rightcolor.g = leftcolor.g;
                  rightcolor.b = leftcolor.b;
                  rightnew.r = leftnew.r;
                  rightnew.g = leftnew.g;
                  rightnew.b = leftnew.b;
                }
              break;
            }
            updateColors(&leftcolor, &rightcolor, &leftnew, &rightnew, mode, isLeft, isFirst,fontsize);
        }
        else
        {
          if (isLeft)
          {
            leftcolor.r = rightcolor.r;
            leftcolor.g = rightcolor.g;
            leftcolor.b = rightcolor.b;
          }
                else
                {
                  rightcolor.r = leftcolor.r;
                  rightcolor.g = leftcolor.g;
                  rightcolor.b = leftcolor.b;
                }
        }
        break;
        case 4:
          if (mode == 2)
          {
            changed = true;
            if (currentColumn == 2)
            {
              changetime += 500000;
              if (changetime == 10500000) changetime = 500000;
            }
            else
            {
              changetime -= 500000;
              if (changetime == 0) changetime = 10000000;
            }
            redrawSetupLine(mode, changetime/100000, selectedsetup, fontsize);
          }
          else
          {
            selectedsetup = currentColumn;
            redrawSetupLine(mode, 0, selectedsetup, fontsize);
            if (selectedsetup == 1) redrawPrefLine(mode, getTextForSetup(selectedsetup), blinktime/100000, fontsize);
            else if (selectedsetup == 2) redrawPrefLine(mode, getTextForSetup(selectedsetup), fadetime/100000, fontsize);
            else redrawPrefLine(mode, getTextForSetup(selectedsetup), shift/100000, fontsize);
          }
        break;
        case 5:
        changed = true;
        if (selectedsetup == 1)
        {
          if (currentColumn == 2)
            {
              blinktime += 100000;
              if (blinktime == 5100000) blinktime = 100000;
            }
            else
            {
              blinktime -= 100000;
              if (blinktime == 0) blinktime = 5000000;
            }
            redrawPrefLine(mode, getTextForSetup(selectedsetup), blinktime/100000, fontsize);
        }
        else if (selectedsetup == 2)
        {
          if (currentColumn == 2)
            {
              fadetime += 100000;
              if (fadetime == 5100000) fadetime = 100000;
            }
            else
            {
              fadetime -= 100000;
              if (fadetime == 0) fadetime = 5000000;
            }
            redrawPrefLine(mode, getTextForSetup(selectedsetup), fadetime/100000, fontsize);
        }
        else
        {
          if (currentColumn == 2)
            {
              shift += 100000;
              if (shift == 5100000) shift = 0;
            }
            else
            {
              shift -= 100000;
              if (shift < 0) shift = 5000000;
            }
            redrawPrefLine(mode, getTextForSetup(selectedsetup), shift/100000, fontsize);
        }
        break;
        case 6:
        if (currentColumn == 3)
        {
          fontsize = fontsize == 2? 1 : 2;
          if (mode == 2) redrawAll(receiver, sender, isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), changetime/100000, selectedsetup);
          else if (mode == 3)
          {
            if (selectedsetup == 1) redrawAll(receiver, sender, isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode,  isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), blinktime/100000, selectedsetup);
            else if (selectedsetup == 2) redrawAll(receiver, sender, isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), fadetime/100000, selectedsetup);
            else redrawAll(receiver, sender, isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), shift/100000, selectedsetup);
          }
          else redrawAll(receiver, sender, isTogether, &leftcolor, &rightcolor, &leftnew, &rightnew, mode, isLeft, isFirst,fontsize, getTextForSetup(selectedsetup), 0, selectedsetup);
        }
        else
        {
          if (sender)
          {
            close(sockfd);
            receiver = false;
            sender = false;
            redrawConnection(receiver, sender, fontsize);
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
      if (!receiver) focus(currentColumn, currentRow, mode);
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
        //end server
        memset(buffer_rx, 0, 60);
        sprintf(strTogether, "%d",isTogether);
        strcat(buffer_rx, strTogether);
        sprintf(strMode, "%d",mode);
        strcat(buffer_rx, strMode);
        sprintf(redLeft, "%02x", leftcolor.r);
        strcat(buffer_rx, redLeft);
        sprintf(greenLeft, "%02x", leftcolor.g);
        strcat(buffer_rx, greenLeft);
        sprintf(blueLeft, "%02x", leftcolor.b);
        strcat(buffer_rx, blueLeft);
        sprintf(redRight, "%02x", rightcolor.r);
        strcat(buffer_rx, redRight);
        sprintf(greenRight, "%02x", rightcolor.g);
        strcat(buffer_rx, greenRight);
        sprintf(blueRight, "%02x", rightcolor.b);
        strcat(buffer_rx, blueRight);
        sprintf(redLeftnew, "%02x", leftnew.r);
        strcat(buffer_rx, redLeftnew);
        sprintf(greenLeftnew, "%02x", leftnew.g);
        strcat(buffer_rx, greenLeftnew);
        sprintf(blueLeftnew, "%02x", leftnew.b);
        strcat(buffer_rx, blueLeftnew);
        sprintf(redRightnew, "%02x", rightnew.r);
        strcat(buffer_rx, redRightnew);
        sprintf(greenRightnew, "%02x", rightnew.g);
        strcat(buffer_rx, greenRightnew);
        sprintf(blueRightnew, "%02x", rightnew.b);
        strcat(buffer_rx, blueRightnew);
        sprintf(intchangetime, "%06x",changetime);
        strcat(buffer_rx, intchangetime);
        sprintf(intblinktime, "%06x", blinktime);
        strcat(buffer_rx, intblinktime);
        sprintf(intfadetime, "%06x", fadetime);
        strcat(buffer_rx, intfadetime);
        sprintf(intblinkshift, "%06x", shift);
        strcat(buffer_rx, intblinkshift);
        printf(buffer_rx);
      sendto(sockfd, buffer_rx, sizeof(buffer_rx), 0, (struct sockaddr*)&braddr, sizeof(braddr));

      printf("\nInfo Sent");
    }


    if (mode == 1)
    {
      *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&leftcolor);
      *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether ? rgbtohex(&leftcolor) : rgbtohex(&rightcolor);
      changed = false;
    }
    else if (mode == 2)
    {
      if (changed) 
      {
      printf("%d,%d,%d\n", leftcolor.r, leftcolor.g, leftcolor.b);
        before = clock();
        *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&leftcolor);
        *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? rgbtohex(&leftcolor) : rgbtohex(&rightcolor);
        gradleft.r = leftcolor.r;
        gradleft.g = leftcolor.g;
        gradleft.b = leftcolor.b;
        gradleftnew.r = leftnew.r;
        gradleftnew.g = leftnew.g;
        gradleftnew.b = leftnew.b;
        gradright.r = rightcolor.r;
        gradright.g = rightcolor.g;
        gradright.b = rightcolor.b;
        gradrightnew.r = rightnew.r;
        gradrightnew.g = rightnew.g;
        gradrightnew.b = rightnew.b;
        changed = false;
      }
      else
      {
        difference = clock() - before;
        if (difference >= changetime)
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
          t.r = gradleft.r + ((double)difference)/changetime*((int)gradleftnew.r - (int)gradleft.r);
          t.g = gradleft.g + ((double)difference)/changetime*((int)gradleftnew.g - (int)gradleft.g);
          t.b = gradleft.b + ((double)difference)/changetime*((int)gradleftnew.b - (int)gradleft.b);
          *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&t);
          if (!isTogether)
          {
            t.r = gradright.r + ((double)difference)/changetime*((int)gradrightnew.r - (int)gradright.r);
            t.g = gradright.g + ((double)difference)/changetime*((int)gradrightnew.g - (int)gradright.g);
            t.b = gradright.b + ((double)difference)/changetime*((int)gradrightnew.b - (int)gradright.b);
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
        beforer = clock() - shift;
        lefton = true;
        // righton = (shift == 0);
        // *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&leftcolor);
        // *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = righton ? (isTogether? rgbtohex(&leftcolor) : rgbtohex(&rightcolor)) : 0x0000;
        righton = true;
        *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&leftcolor);
        *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? rgbtohex(&leftcolor) : rgbtohex(&rightcolor);
        changed = false;
      }
      else
      {
        difference = clock() - before;
        differencer = clock() - beforer;
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
            *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgbtohex(&leftcolor);
            if (shift == 0) 
            {
              *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? rgbtohex(&leftcolor) : rgbtohex(&rightcolor);
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
                *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? rgbtohex(&leftcolor) : rgbtohex(&rightcolor);
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
