#include "broadcastcontrol.h"
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

bool updateDataBool(bool* data, char* buffer_rx, char * offset, const char datasize)
{
    bool tempbool;
    char temp[datasize];
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
    int tempint;
    char temp[datasize];
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


void prepareData(State* state, char* buffer_rx)
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