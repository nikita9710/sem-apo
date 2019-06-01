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
  while (1)
  {
    rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    bk =  rgb_knobs_value      & 0xFF; // blue knob position
    gk = (rgb_knobs_value>>8)  & 0xFF; // green knob position
    rk = (rgb_knobs_value>>16) & 0xFF; // red knob position
    bb = (rgb_knobs_value>>24) & 1;    // blue button
    gb = (rgb_knobs_value>>25) & 1;    // green button
    rb = (rgb_knobs_value>>26) & 1;    // red buttom


  bool isTogether = false, changed = true;
  int leftcolor, rightcolor, mode; //0 - still, 1 - gradient, 2 - blinkkink
  int leftnew, rightnew, changetime;
  bool reverse = false, lefton = true, righton = true;
  int blinktime, fadetime, shift;

  // if (!changed) continue;
  if (mode == 1)
  {
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = leftcolor;
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? leftcolor : rightcolor;
  }
  else if (mode == 2)
  {
    if (changed) 
    {
      before = clock();
      reverse = false;
      *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = leftcolor;
      *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? leftcolor : rightcolor;
    }
    else
    {
      difference = clock() - before;
      if (difference >= changetime)
      {
        if (reverse)
        {
          *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = leftcolor;
          *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? leftcolor : rightcolor;
        }
        else
        {
          *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = leftnew;
          *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? leftnew : rightnew;
        }
        reverse = !reverse;
        before = clock();
      }
      else
      {
        //average colors??????????
      }
    }
  }
  else
  {
    if (changed) 
    {
      before = clock();
      lefton = true;
      righton = true;
      *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = leftcolor;
      *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? leftcolor : rightcolor;
    }
    else
    {
      difference = clock() - before;
      int differencer = clock() - before - shift;
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
          *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = leftcolor;
          if (shift == 0) 
          {
            *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? leftcolor : rightcolor;
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
            if (difference >= blinktime)
            {
              *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = 0x0;
              righton = false;
              before = clock();
            }
          }
          else
          {
            if (difference >= fadetime)
            {
              *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? leftcolor : rightcolor;
              righton = true;
              before = clock();
            }
          }
      }
      if (difference >= changetime)
      {
        if (reverse)
        {
          *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = leftcolor;
          *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? leftcolor : rightcolor;
        }
        else
        {
          *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = leftnew;
          *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = isTogether? leftnew : rightnew;
        }
        reverse = !reverse;
        before = clock();
      }
      else
      {
        //average colors??????????
      }
    }
  }
  
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
