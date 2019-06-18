
#ifndef SEM_APO_FINAL_H
#define SEM_APO_FINAL_H
#include "broadcastcontrol.h"
#include "screencontrol.h"
#define _POSIX_C_SOURCE 200112L
#define LOCAL_PORT 8888

void resetState(State* state);

void increasecolor(RGB* color, char dr, char dg, char db);

int rgbtohex(RGB* rgb);
#endif /*SEM_APO_FINAL_H*/