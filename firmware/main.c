/*
 * main.c
 *
 *  Created on: May 22, 2014
 *      Author: lim
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "ports.h"
#include "macro.h"
#include "usart.h"
#include "i2c.h"
#include "ds3231.h"
#include "ws2812.h"

#define SND PORT_D2

#define DS3231_IN32KHZ PORT_C3 //PCINT11
#define DS3231_INSQW PORT_D4 //PCINT20
#define DS3231_RST PORT_D3

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

void hsvToGrb(uint8_t *hsv, uint8_t *grb) {
	uint8_t h = hsv[0];
	uint8_t s = hsv[1];
	uint8_t v = hsv[2];

	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;

	if (s > 0) {
		uint8_t chroma = (uint8_t)((uint16_t)(v*s)/255);
		switch (h) {
			case 0 ... 43:
				r = chroma;
				g = (chroma * h) / 43;
				break;
			case 44 ... 85:
				r = (chroma * (85-h)) / 42;
				g = chroma;
				break;
			case 86 ... 128:
				g = chroma;
				b = (chroma * (h-86)) / 43;
				break;
			case 129 ... 171:
				g = (chroma * (171-h)) / 43;
				b = chroma;
				break;
			case 172 ... 213:
				r = (chroma * (h-172)) / 42;
				b = chroma;
				break;
			case 214 ... 255:
				r = chroma;
				b = (chroma * (255-h)) / 43;
				break;
			default:
				break;
		}

		uint8_t m = v - chroma;
		r += m;
		g += m;
		b += m;
	} else {
		r = v;
		g = v;
		b = v;
	}
	grb[1] = r;
	grb[0] = g;
	grb[2] = b;
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

	SETD(HC4052_A);
	CLRP(HC4052_A);
	SETD(HC4052_B);
	CLRP(HC4052_B);
	SETD(HC4052_INH);
	CLRP(HC4052_INH);

	SETP(DS3231_IN32KHZ); //set pull-up resistor
	CLRD(DS3231_IN32KHZ);
	SETP(DS3231_INSQW); //set pull-up resistor
	CLRD(DS3231_INSQW);
	SETP(DS3231_RST);
	SETD(DS3231_RST);

	SETD(SND);

	clrscr();
	refresh();

	for (uint8_t i = 0; i < P_TOTAL; i++) {
		uint8_t hsv[3] = {(i*255)/P_TOTAL, 255, 16};
		hsvToGrb(hsv, &(pattern[i*3]));
//		pattern[i*3+0] = 16; //G
//		pattern[i*3+1] = 32; //R
//		pattern[i*3+2] = 2; //B
//		pattern[i*3+0] = rand() % 16; //G
//		pattern[i*3+1] = rand() % 16; //R
//		pattern[i*3+2] = rand() % 16; //B
	}

//	for (uint8_t i = 0; i < P_TOTAL; i++) {
//		uint8_t x = i % 5;
//		uint8_t y = i / 5;
//		if (x<2) {
//			pattern[i*3+0] = 3; //G
//			pattern[i*3+1] = 10; //R
//			pattern[i*3+2] = 0; //B
//		} else if (x>2) {
//			pattern[i*3+0] = 0; //G
//			pattern[i*3+1] = 0; //R
//			pattern[i*3+2] = 4; //B
//		} else if (y%2) {
//			pattern[i*3+0] = 3; //G
//			pattern[i*3+1] = 10; //R
//			pattern[i*3+2] = 4; //B
//		} else {
//			pattern[i*3+0] = 3; //G
//			pattern[i*3+1] = 10; //R
//			pattern[i*3+2] = 4; //B
//		}
//	}

	usart_printstr("\n\rp-clock\n\r");

//	pattern[0*3+1] = 255;

//	ds3231_settime(ttime(12, 58, 30), 0);
//	ds3231_setdate(tdate(2, 11, 11, 15), 0);
}

void handle_uart() {
	char ch = usart_getchr();
	switch (ch) {
		case 'h':
			{
				usart_printstr("help");
				usart_printstr("\n\r"
						"h - help\n\r"
						"txxxxxx - set time\n\r"
						"wxx - set worktime\n\r"
						"rxx - set resttime\n\r"
						"mx - mode\n\r"
						"s - start work\n\r"
						"a - start rest\n\r"
//						"fxxxx - set fade delay\n\r"
//						"cyxxxxxx - set color for k\n\r"
						);
				break;
			}
		case 't':
			{
				usart_printstr("time");
				uint8_t hours = usart_hextochar(usart_getchr())*10;
				hours += usart_hextochar(usart_getchr());
				usart_printhex(hours);
				uint8_t minutes = usart_hextochar(usart_getchr())*10;
				minutes += usart_hextochar(usart_getchr());
				usart_printhex(minutes);
				uint8_t seconds = usart_hextochar(usart_getchr())*10;
				seconds += usart_hextochar(usart_getchr());
				usart_printhex(seconds);
				TTime time = ttime(hours, minutes, seconds);
				ds3231_settime(time, 0);
				break;
			}
		default:
			break;
	}
	usart_printstr("\n\rok\n\r");
}

void loop() {
//	INVP(SND);
	static uint8_t tt = 0;
	tt++;
	clrscr();

	if (usart_chrready()) {
		handle_uart();
	}
//	for (uint8_t i = 0; i < P_TOTAL; i++) {
//		pattern[i*3+0] = rand() % 16; //G
//		pattern[i*3+1] = rand() % 16; //R
//		pattern[i*3+2] = rand() % 16; //B
//	}

//	//shift left
//	for (uint8_t y = 0; y < P_HEIGHT; y++) {
//		uint8_t tmp0 = pattern[(y * P_HEIGHT + 0) * 3 + 0];
//		uint8_t tmp1 = pattern[(y * P_HEIGHT + 0) * 3 + 1];
//		uint8_t tmp2 = pattern[(y * P_HEIGHT + 0) * 3 + 2];
//
//		for (uint8_t x = 1; x < P_WIDTH; x++) {
//			pattern[(y * P_HEIGHT + x - 1) * 3 + 0] = pattern[(y * P_HEIGHT + x) * 3 + 0];
//		}
//		pattern[(y * P_HEIGHT + P_WIDTH - 1) * 3 + 0] = tmp0;
//		pattern[(y * P_HEIGHT + P_WIDTH - 1) * 3 + 1] = tmp1;
//		pattern[(y * P_HEIGHT + P_WIDTH - 1) * 3 + 2] = tmp2;
//	}


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

	uint8_t sec = dectobin(time.sec);
	current_pixels[(8*5+sec/12)*3+0] = (sec & 1) ? 16 : 0;
	current_pixels[(8*5+sec/12)*3+1] = (sec & 2) ? 16 : 0;
	current_pixels[(8*5+sec/12)*3+2] = (sec & 4) ? 16 : 0;

//	uint8_t i = rand() % P_TOTAL;
//	uint8_t c = rand() % 7 + 1;
//	current_pixels[i*3+0] = (c & 1) ? 255 : 0;
//	current_pixels[i*3+1] = (c & 2) ? 255 : 0;
//	current_pixels[i*3+2] = (c & 4) ? 255 : 0;

	refresh();
	if ((tt&15) == 1) {
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

//	i = rand() % P_TOTAL;
//	c = rand() % 7 + 1;
//	pattern[i*3+0] = (c & 1) ? 16 : 0;
//	pattern[i*3+1] = (c & 2) ? 16 : 0;
//	pattern[i*3+2] = (c & 4) ? 16 : 0;
}

int main(void)
{
	init();
	while (1) {
		loop();
	}
	return 0;
}

