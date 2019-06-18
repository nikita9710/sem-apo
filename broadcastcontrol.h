#ifndef BROADCASTCONTROL_H
#define BROADCASTCONTROL_H
#include <stdbool.h>
#include "screencontrol.h"

bool updateDataBool(bool* data, char* buffer_rx, char * offset, const char datasize);

bool updateDataInt(int* data, char* buffer_rx, char * offset, const char datasize);

bool updateDataChar(char* color, char* buffer_rx, char * offset, const char datasize);

bool updateColor(RGB* color, char* buffer_rx, char * offset);

bool updateState(State* state, char* buffer_rx);

void prepareData(State* state, char* buffer_rx);
#endif /*BROADCASTCONTROL_H*/
