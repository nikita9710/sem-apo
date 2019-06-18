#define _POSIX_C_SOURCE 200112L
#include "screencontrol.h"
#define LOCAL_PORT 8888

void resetState(State* state);

void increasecolor(RGB* color, char dr, char dg, char db);

int rgbtohex(RGB* rgb);