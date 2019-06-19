
#ifndef BROADCASTCONTROL_H
#define BROADCASTCONTROL_H
#include <stdbool.h>
#include "screencontrol.h"
/**
 * @file    broadcastcontrol.h
 * @author  Kamila Babayeva, Nikita  Milyukov
 *
 */
/**
 * @brief   Convert received hex value to boolean and check if the value is changed.
 * @param   data      values to change
 * @param   buffer_rx    received data
 * @param   offset     counter to distinguish data
 * @param   datasize     the size of one value in received data
 * @return  @c true if the data is changed, otherwise return @c false
 */
bool updateDataBool(bool* data, char* buffer_rx, char * offset, const char datasize);
/**
 * @brief   Convert received hex value to int and check if the value is changed.
 * @param   data      values to change
 * @param   buffer_rx    received data
 * @param   offset     counter to distinguish data
 * @param   datasize     the size of one value in received data
 * @return  @c true if the data is changed, otherwise return @c false
 */
bool updateDataInt(int* data, char* buffer_rx, char * offset, const char datasize);
/**
 * @brief   Convert received hex value to char and check if the value is changed.
 * @param   data      values to change
 * @param   buffer_rx    received data
 * @param   offset     counter to distinguish data
 * @param   datasize     the size of one value in received data
 * @return  @c true if the data is changed, otherwise return @c false
 */
bool updateDataChar(char* color, char* buffer_rx, char * offset, const char datasize);
/**
 * @brief   Combine all received colors for one reflector.
 * @param   color      parameters of RGB structure
 * @param   buffer_rx    received data
 * @param   offset     counter to distinguish data 
 * @return  @c true if the data is changed, otherwise return @c false
 */
bool updateColor(RGB* color, char* buffer_rx, char * offset);
/**
 * @brief   Combine all received data to change.
 * @param   state      parameters of State structure
 * @param   buffer_rx    received data
 * @return  @c true if the data is changed, otherwise return @c false
 */
bool updateState(State* state, char* buffer_rx);
/**
 * @brief   Combine all data to be sent
 * @param   state      parameters of State structure
 * @param   buffer_rx    sent data
 */
void prepareData(State* state, char* buffer_rx);
#endif /*BROADCASTCONTROL_H*/
