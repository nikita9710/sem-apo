
#ifndef SEM_APO_FINAL_H
#define SEM_APO_FINAL_H
#include "broadcastcontrol.h"
#include "screencontrol.h"
#define _POSIX_C_SOURCE 200112L
#define LOCAL_PORT 8888
/**
 * @file    sem_apo_final.h
 * @author  Kamila Babayeva, Nikita  Milyukov
 *
 */

/**
 * @brief   Reset the parameters in the structure State to initial ones
 * @param   state      structure State
 */
void resetState(State* state);
/**
 * @brief   change the appropriate color after turning the knobs
 * @param   state      structure State
 * @param   dr      the state of red knob
 * @param   dg      the state of green knob
 * @param   db      the state of blue knob
 */
void increasecolor(RGB* color, char dr, char dg, char db);
/**
 * @brief   Converter from RGB to hex for reflectors.
 * @param   RGB      RGB Structure of colors
 * @return   @a Rgb structure in hex
 */
int rgbtohex(RGB* rgb);
#endif /*SEM_APO_FINAL_H*/
