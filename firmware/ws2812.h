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

#define LED_CTRL0 PORT_D5
#define LED_CTRL1 PORT_D6
#define LED_CTRL2 PORT_D7
#define LED_CTRL3 PORT_B0
#define LED_CTRL4 PORT_B1
#define LED_CTRL5 PORT_B2

void refresh();

#define P_WIDTH 17
#define P_HEIGHT 5
#define P_TOTAL (P_WIDTH*P_HEIGHT)
#define D_WIDTH 3
#define D_HEIGHT 5
#define D_TOTAL (D_WIDTH*D_HEIGHT)

uint8_t current_pixels[P_TOTAL*3];
uint8_t new_pixels[P_TOTAL*3];
uint8_t pattern[P_TOTAL*3];

void setnum(uint8_t pos, uint8_t num);

#endif /* WS2812_H_ */
