#ifndef SCREENCONTROL_H

#include <stdbool.h>

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

int toRGB565(char red, char green, char blue);

void blockColorChange(int blockX, int blockY, int blockXrange, int blockYrange,  int color);

void background(int blockX, int blockY, int color, int blockXsize, int delta);

void chosenBorder(int blockX, int blockY, int color, bool isDoubled, int delta);

void fillBlock(char* str, int blockX, int blockY, int blockXsize, int blockYsize, int size );

void setfocus(int col, int row, int oldcol, int oldrow, int mode);

void focus(int col, int row, int mode);

void focusColor(int col, int row);

void updateColumn(State* state, int* currentRow, int* currentColumn);

void updateRow(State* state, int* currentRow, int* currentColumn);

void updateColors(State* state, bool isLeft, bool isFirst, char fontsize);

void setCopyButton(bool isOn, char fontsize);

void redrawLEDLine(bool isTogether, bool isLeft, char fontsize);

void redrawModeLine(int mode, char fontsize);

void redrawSetupLine(int mode, int time, int selectedsetup, char fontsize);

char* getTextForSetup(int setup);

void redrawPrefLine(State* state, int selectedsetup, char fontsize);

void redrawConstants(bool isSender, char fontsize);

void redrawConnection(bool sender, char fontsize);

void redrawAll(State * state, bool isSender,  bool isLeft, bool isFirst, char fontsize, int selectedsetup);

void drawReceiver();

void drawScreen(unsigned char * parlcd_mem_base);

#endif