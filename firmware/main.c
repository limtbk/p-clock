/*
 * main.c
 *
 *  Created on: May 22, 2014
 *      Author: lim
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "ports.h"
#include "macro.h"
#include "usart.h"
#include "i2c.h"
#include "ds3231.h"
#include "ws2812.h"

#define SND PORT_D2

#define DS3231_IN32KHZ PORT_C3

#define HC4052_A PORT_C2
#define HC4052_B PORT_C1
#define HC4052_INH PORT_C0

void clrscr() {
	for (uint8_t i = 0; i<P_TOTAL*3; i++) {
		new_pixels[i] = 0;
		current_pixels[i] = 0;
	}
}

//void gotocurrent() {
//	for (uint8_t i = 0; i<54*3; i++) {
//		if (current[i]<pixels[i]) {
//			current[i] = current[i] + 1;
//			finished = 0;
//		} else if (current[i]>pixels[i]) {
//			current[i] = current[i] - 1;
//			finished = 0;
//		}
//	}
//}

void initTimer()
{
	TCCR1A = 0; //No compare
	TCCR1B = (1<<WGM12)|(0b101<<CS10); //CTC, clk/1024
	OCR1A = 15625+1; //16000000 / 1024 = 15625 (7812+7813)
	TIMSK1 = (1<<OCIE1A);
}

ISR(TIMER1_COMPA_vect)
{
}


void init() {
	usart_init();
	i2c_init();
	initTimer();
	SETD(LED_CTRL0);
	CLRP(LED_CTRL0);
	SETD(LED_CTRL1);
	CLRP(LED_CTRL1);
	SETD(LED_CTRL2);
	CLRP(LED_CTRL2);
	SETD(LED_CTRL3);
	CLRP(LED_CTRL3);
	SETD(LED_CTRL4);
	CLRP(LED_CTRL4);
	SETD(LED_CTRL5);
	CLRP(LED_CTRL5);

	clrscr();
	refresh();

	for (uint8_t i = 0; i < P_TOTAL; i++) {
		pattern[i*3+0] = 2;
		pattern[i*3+1] = 2;
		pattern[i*3+2] = 2;
	}

	pattern[0*3] = 1;
	pattern[5*3] = 2;
	pattern[10*3] = 4;
	pattern[15*3] = 8;
	pattern[20*3] = 16;
	pattern[25*3] = 32;
	pattern[30*3] = 64;
	pattern[35*3] = 128;
	pattern[30*3+2] = 1;
	pattern[35*3+2] = 2;
	pattern[40*3+2] = 4;
	pattern[45*3+2] = 8;
	pattern[50*3+2] = 16;
	pattern[55*3+2] = 32;
	pattern[60*3+2] = 64;
	pattern[65*3+2] = 128;
	pattern[60*3+1] = 1;
	pattern[65*3+1] = 2;
	pattern[70*3+1] = 4;
	pattern[75*3+1] = 8;
	pattern[80*3+1] = 16;
	pattern[85*3+1] = 32;
	pattern[0*3+1] = 64;
	pattern[5*3+1] = 128;

//	ds3231_settime(ttime(02, 50, 40), 0);
//	ds3231_setdate(tdate(2, 4, 11, 15), 0);
}

void loop() {
	clrscr();

	TTime time;
	TDate date;

	ds3231_gettime(&time, 1);
	ds3231_getdate(&date, 1);

//	setnum(0, time.sec & 0x0F);
//	setnum(1, (time.sec & 0xF0) >> 4);
//	setnum(2, time.min & 0x0F);
//	setnum(3, (time.min & 0xF0) >> 4);

	setnum(0, time.min & 0x0F);
	setnum(1, (time.min & 0xF0) >> 4);
	setnum(2, time.hour & 0x0F);
	setnum(3, (time.hour & 0xF0) >> 4);


	refresh();
	uint8_t tmp0 = pattern[0*3+0];
	uint8_t tmp1 = pattern[0*3+1];
	uint8_t tmp2 = pattern[0*3+2];
	for (uint8_t i = 0; i < P_TOTAL; i++) {
		pattern[i*3+0] = pattern[(i+1)*3+0];
		pattern[i*3+1] = pattern[(i+1)*3+1];
		pattern[i*3+2] = pattern[(i+1)*3+2];
	}
	pattern[84*3+0] = tmp0;
	pattern[84*3+1] = tmp1;
	pattern[84*3+2] = tmp2;
}

int main(void)
{
	init();
	while (1) {
		loop();
	}
	return 0;
}

