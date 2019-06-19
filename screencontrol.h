#ifndef SCREENCONTROL_H
#define SCREENCONTROL_H
#include <stdbool.h>

/**
 * @file    screencontrol.h
 * @author  Kamila Babayeva, Nikita  Milyukov
 *
 */

 /**
 * @brief   RGB color structure.
 */
typedef struct 
{
  char r; /**< red color        */
  char g; /**< green color        */
  char b; /**< blue color        */
} RGB;

/**
 * @brief parameters used to implement sender and receiver modes.
 */
typedef struct
{
  int mode; /**< mode: still, gradient or blink       */
  bool isTogether; /**< both reflecters are on or not        */
  RGB leftcolor, rightcolor, leftnew, rightnew; /**< previous and current colors for both reflectors     */
  int changetime; /**< duration during gradient mode        */
  int blinktime, fadetime, shift; /**< settings during blink mode        */
} State;

/**
 * @brief   Converter from RGB to RGB565.
 * @param   red      red color
 * @param   green    green color
 * @param   blue     blue color
 * @return  @a red,@a green,@a blue  in RGB565 
 */
int toRGB565(char red, char green, char blue);

/**
 * @brief   Change the color of specified block on the display
 * @param   blockX   X coordinate of the block on the display
 * @param   blockY   Y coordinate of the block on the display
 * @param   blockXrange      the block length on the display
 * @param   blockXrange      the block width on the display
 * @param   color      the backgrounf color of the block
 */
void blockColorChange(int blockX, int blockY, int blockXrange, int blockYrange,  int color);
/**
 * @brief   Change the backgound color of specified block on the display according to the color of reflector
 * @param   blockX   X coordinate of the block on the display
 * @param   blockY   Y coordinate of the block on the display
 * @param   blockXsize      the length of the block
 * @param   color      the  background color of the bolck
 * @param   delta      space to draw the borders
 */
void background(int blockX, int blockY, int color, int blockXsize, int delta);
/**
 * @brief   Change the color of borders in chosen block on the display 
 * @param   blockX   X coordinate of the block on the display
 * @param   blockY   Y coordinate of the block on the display
 * @param   isDoubled      Check if two blocks are joint
 * @param   delta     space to draw the borders
 * @param   color      the border color of the block
 */
void chosenBorder(int blockX, int blockY, int color, bool isDoubled, int delta);
/**
 * @brief   Write the text in specified block on the display
 * @param   blockX   X coordinate of the block on the display
 * @param   blockY   Y coordinate of the block on the display
 * @param   blockXrange      the block length on the display
 * @param   blockXrange      the block width on the display
 * @param   size      font size
 */
void fillBlock(char* str, int blockX, int blockY, int blockXsize, int blockYsize, int size );
/**
 * @brief   Set the focus of a block on the display
 * @param   col   current chosen column
 * @param   row   current chosen row
 * @param   oldcol      previous chosen column
 * @param   oldrow     previous chosen row
 * @param   mode      check mode
 */
void setfocus(int col, int row, int oldcol, int oldrow, int mode);
/**
 * @brief   Set the focus of a block on the display
 * @param   col   current chosen column
 * @param   row   current chosen row
 * @param   mode      pass the chosen mode
 */
void focus(int col, int row, int mode);
/**
 * @brief   Save the focus of a block on the display
 * @param   col   chosen column
 * @param   row   chosen row
 */
void focusColor(int col, int row);
/**
 * @brief   Сhange the block in a column on the display according to the pressed knob
 * @param   state   parameters in the structure
 * @param   currentRow   сurrent row
 * @param   currentColumn      current column
 */
void updateColumn(State* state, int* currentRow, int* currentColumn);
/**
 * @brief   Сhange the block in a row on the display according to the pressed knob
 * @param   state   parameters in the structure
 * @param   currentRow   сurrent row
 * @param   currentColumn      current column
 */
void updateRow(State* state, int* currentRow, int* currentColumn);
/**
 * @brief   Change the color of the background in the color block on the display according to the reflectors' color
 * @param   state   parameters in structure state
 * @param   isLeft   check the chosen reflector
 * @param   isFirst      check if the first color to change is chosen (used in a gradient mode)
 * @param   fontsize      check the chosen fontsize
 */
void updateColors(State* state, bool isLeft, bool isFirst, char fontsize);
/**
 * @brief   draw the Copy button on the display if needed
 * @param   isOn   check if the Copy button shold be on the display (depends on the mode)
 * @param   fontsize      check the chosen fontsize
 */
void setCopyButton(bool isOn, char fontsize);
/**
 * @brief   redraw Led row on the display
 * @param   isTogether   check if both reflectors are chosen
 * @param   isLeft   check the chosen reflector
 * @param   fontsize      check the chosen fontsize
 */
void redrawLEDLine(bool isTogether, bool isLeft, char fontsize);
/**
 * @brief   redraw Mode row on the display
 * @param   mode   check if both reflectors are chosen
 * @param   fontsize      check the chosen fontsize
 */
void redrawModeLine(int mode, char fontsize);
/**
 * @brief   redraw Setup row on the display
 * @param   mode   check if both reflectors are chosen
 * @param   time   check if both reflectors are chosen
 * @param   selectedSetup   check if both reflectors are chosen
 * @param   fontsize      check the chosen fontsize
 */
void redrawSetupLine(int mode, int time, int selectedsetup, char fontsize);
/**
 * @brief   change the text for the following setup
 * @param   setup   chosen setup
 */
char* getTextForSetup(int setup);
/**
 * @brief   choose the time mode according to the choosen setup 
 * @param   state   parameters in structure state
 * @param   selectedSetup   chosen setup in Setup row
 * @param   fontsize      check the chosen fontsize
 * @return  string of selected time mode
 */
void redrawPrefLine(State* state, int selectedsetup, char fontsize);
/**
 * @brief   draw the constant blocks on the display (first left column)
 * @param   isSender   check the board mode
 * @param   fontsize      check the chosen fontsize
 */
void redrawConstants(bool isSender, char fontsize);
/**
 * @brief   redraw the display according to the chosen mode
 * @param   sender   check the board mode
 * @param   fontsize      check the chosen fontsize
 */
void redrawConnection(bool sender, char fontsize);
/**
 * @brief   redraw the display according to the chosen mode
 * @param   state      parameters of structure state
 * @param   isSender   check the board mode
 * @param   isLeft      check the chosen reflector
 * @param   isFirst      check if the first color to change is chosen (used in a gradient mode)
 * @param   fontsize      check the chosen fontsize
 * @param   selectedsetup      chosen setup in Setup row
 */
void redrawAll(State * state, bool isSender,  bool isLeft, bool isFirst, char fontsize, int selectedsetup);
/**
 * @brief   redraw the display if the board is in the receiver mode
 */
void drawReceiver();
/**
 * @brief   store the display image in a matrix and print it out on the display 
 * @param   parlcd_mem_base      memory address of LCD display
*/
void drawScreen(unsigned char *parlcd_mem_base);
#endif /*SCREENCONTROL_H*/
