#include "sem_apo_final.h"
#include "broadcastcontrol.h"
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

int rgbtohex(RGB* rgb)
{
  return ((rgb->r & 0xff) << 16) + ((rgb->g & 0xff) << 8) + (rgb->b & 0xff);
}

void increasecolor(RGB* color, char dr, char dg, char db)
{
  color->r += dr;
  color->g += dg;
  color->b += db;
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

      drawScreen(parlcd_mem_base);
    }

    if (changed && sender)
    {
      prepareData(&state, buffer_rx);
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