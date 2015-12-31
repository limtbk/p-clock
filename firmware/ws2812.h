/*
 * ws2812.h
 *
 *  Created on: Nov 4, 2015
 *      Author: lim
 */

#ifndef WS2812_H_
#define WS2812_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include "ports.h"
#include "macro.h"

#define LED_CTRL PORTA_D8

void refresh();

//#define D_WIDTH 3
//#define D_HEIGHT 5
#define D_TOTAL (15-2)
//#define P_WIDTH 17
//#define P_HEIGHT 5
#define P_TOTAL (D_TOTAL*4+2)

uint8_t current_pixels[P_TOTAL*3];
uint8_t new_pixels[P_TOTAL*3];
uint8_t pattern[P_TOTAL*3];

void setnum(uint8_t pos, uint8_t num);
void setchr(uint8_t pos, uint8_t num);
void hsvToGrb(uint8_t *hsv, uint8_t *grb);

#endif /* WS2812_H_ */
