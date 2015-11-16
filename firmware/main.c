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

#define DS3231_IN32KHZ PORT_C3 //PCINT11 on PCIE1
#define DS3231_INSQW PORT_D4 //PCINT20 on PCIE2
#define DS3231_RST PORT_D3

#define HC4052_A PORT_C2
#define HC4052_B PORT_C1
#define HC4052_INH PORT_C0

typedef struct ClockStatusStruct {
	uint8_t mode;
	uint8_t bkmode;
	TTime time;
	TDate date;
	TTime workTimeStart;
	TTime restTimeStart;
	uint8_t workTimeMinutes;
	uint8_t restTimeMinutes;
} TClockStatus;

TClockStatus cSt;

void clrscr() {
	for (uint8_t i = 0; i<P_TOTAL*3; i++) {
		new_pixels[i] = 0;
		current_pixels[i] = 0;
	}
}

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

ISR(PCINT0_vect) {
	static uint16_t t = 0;
	t++;
	if (t == 0) {
//		usart_putchr('0');
	}
}

ISR(PCINT1_vect) {
	static uint32_t t = 0;
	static uint8_t s = 0;

	t++;
	if (s != cSt.time.sec) {
		s = cSt.time.sec;
//		usart_printhex(t>>24);
//		usart_printhex(t>>16);
//		usart_printhex(t>>8);
//		usart_printhex(t);
//		usart_printstr("\n\r");
		t = 0;
	}
//	if (t == 32768) {
//		t = 0;
////		usart_putchr('1');
//	}
}

ISR(PCINT2_vect) {
	static uint16_t t = 0;
	t++;
	if (t == 0) {
//		usart_putchr('2');
	}
}

void init() {
	usart_init();
	i2c_init();
//	initTimer();
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

	PCICR |= bv(PCIE1) | bv(PCIE2);
	PCMSK1 |= bv(PCINT11);
	PCMSK2 |= bv(PCINT20);

	clrscr();
	refresh();

	for (uint8_t i = 0; i < P_TOTAL; i++) {
		uint8_t hsv[3] = {(i*255)/P_TOTAL, 255, 16};
		hsvToGrb(hsv, &(pattern[i*3]));
	}

	cSt.mode = 0;
	cSt.bkmode = 0;
	cSt.workTimeMinutes = 45;
	cSt.restTimeMinutes = 5;

	usart_printstr("\n\rp-clock\n\r");
}

void startwork() {
	cSt.workTimeStart.hour = cSt.time.hour;
	cSt.workTimeStart.min = cSt.time.min;
	cSt.workTimeStart.sec = cSt.time.sec;
	cSt.mode = 2;
	cSt.bkmode = 1;
	usart_printstr("\n\rstart work\n\r");

}

void startrest() {
	cSt.restTimeStart.hour = cSt.time.hour;
	cSt.restTimeStart.min = cSt.time.min;
	cSt.restTimeStart.sec = cSt.time.sec;
	cSt.mode = 3;
	cSt.bkmode = 2;
	usart_printstr("\n\rstart rest\n\r");
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
						"dxxxxxx - set date\n\r"
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
		case 'd':
			{
				usart_printstr("date");
				uint8_t year = usart_hextochar(usart_getchr())*10;
				year += usart_hextochar(usart_getchr());
				usart_printhex(year);
				uint8_t month = usart_hextochar(usart_getchr())*10;
				month += usart_hextochar(usart_getchr());
				usart_printhex(month);
				uint8_t day = usart_hextochar(usart_getchr())*10;
				day += usart_hextochar(usart_getchr());
				usart_printhex(day);
				TDate date = tdate(0, day, month, year);
				ds3231_setdate(date, 0);
				break;
			}
		case 'm':
			{
				usart_printstr("mode");
				cSt.mode = usart_hextochar(usart_getchr());
				usart_printhex(cSt.mode);
				if (cSt.mode == 0) {
					cSt.bkmode = 0;
				}
				if (cSt.mode == 1) {
					cSt.bkmode = 0;
				}
				if (cSt.mode == 2) {
					cSt.bkmode = 1;
				}
				if (cSt.mode == 3) {
					cSt.bkmode = 2;
				}
				break;
			}
		case 's':
			{
				startwork();
				break;
			}
		case 'a':
			{
				startrest();
				break;
			}
		case 'k':
			{
				usart_printstr("colormode");
				cSt.bkmode = usart_hextochar(usart_getchr());
				usart_printhex(cSt.mode);
				break;
			}
		case 'w':
			{
				usart_printstr("worktime");
				cSt.workTimeMinutes = usart_hextochar(usart_getchr())*10;
				cSt.workTimeMinutes += usart_hextochar(usart_getchr());
				usart_printhex(cSt.workTimeMinutes);
				break;
			}
		case 'r':
			{
				usart_printstr("resttime");
				cSt.restTimeMinutes = usart_hextochar(usart_getchr())*10;
				cSt.restTimeMinutes += usart_hextochar(usart_getchr());
				usart_printhex(cSt.restTimeMinutes);
				break;
			}

		default:
			break;
	}
	usart_printstr("\n\rok\n\r");
}

void rainbowcycling() {
	static uint8_t tt = 0;
	tt++;
	for (uint8_t i = 0; i < P_TOTAL; i++) {
		uint8_t hsv[3] = {((i+tt)*255)/P_TOTAL, 255, 16};
		hsvToGrb(hsv, &(pattern[i*3]));
	}
}

void setbkcolor(uint8_t color) {
	for (uint8_t i = 0; i < P_TOTAL; i++) {
		uint8_t hsv[3] = {color, 255, 16};
		hsvToGrb(hsv, &(pattern[i*3]));
	}
}

void showclock() {
	setnum(0, cSt.time.min & 0x0F);
	setnum(1, (cSt.time.min & 0xF0) >> 4);
	setnum(2, cSt.time.hour & 0x0F);
	setnum(3, (cSt.time.hour & 0xF0) >> 4);

	uint8_t sec = dectobin(cSt.time.sec);
	uint8_t ss = sec % 12 + 2;
	current_pixels[(8*5+sec/12)*3+0] = ((ss & 2) ? 16 : 0) * (ss & 1);
	current_pixels[(8*5+sec/12)*3+1] = ((ss & 4) ? 16 : 0) * (ss & 1);
	current_pixels[(8*5+sec/12)*3+2] = ((ss & 8) ? 16 : 0) * (ss & 1);
}

void showseconds() {
	setnum(0, cSt.time.sec & 0x0F);
	setnum(1, (cSt.time.sec & 0xF0) >> 4);
	setnum(2, cSt.time.min & 0x0F);
	setnum(3, (cSt.time.min & 0xF0) >> 4);
}

void showdate() {
	setnum(0, cSt.date.day & 0x0F);
	setnum(1, (cSt.date.day & 0xF0) >> 4);
	setnum(2, cSt.date.month & 0x0F);
	setnum(3, (cSt.date.month & 0xF0) >> 4);
}

void showyear() {
	setnum(0, cSt.date.year & 0x0F);
	setnum(1, (cSt.date.year & 0xF0) >> 4);
	setnum(2, 0);
	setnum(3, 2);
}

void showwork() {
	int16_t ttime = timetoseconds(cSt.time);
	int16_t dtime = timetoseconds(cSt.workTimeStart);
	int16_t wtime = cSt.workTimeMinutes*60;

	int16_t rtime = ttime - dtime;
	if ((rtime<0) || (rtime>wtime)) {
		startrest();
	} else {
		TTime timer;
		secondstotime(wtime-rtime, &timer);
		setnum(0, timer.sec & 0x0F);
		setnum(1, (timer.sec & 0xF0) >> 4);
		setnum(2, timer.min & 0x0F);
		setnum(3, (timer.min & 0xF0) >> 4);
	}
}

void showrest() {
	int16_t ttime = timetoseconds(cSt.time);
	int16_t dtime = timetoseconds(cSt.restTimeStart);
	int16_t wtime = cSt.restTimeMinutes*60;

	int16_t rtime = ttime - dtime;
	if ((rtime<0) || (rtime>wtime)) {
		startwork();
	} else {
		TTime timer;
		secondstotime(wtime-rtime, &timer);
		setnum(0, timer.sec & 0x0F);
		setnum(1, (timer.sec & 0xF0) >> 4);
		setnum(2, timer.min & 0x0F);
		setnum(3, (timer.min & 0xF0) >> 4);
	}
}

void loop() {
//	INVP(SND);
	static uint8_t tt = 0;
	tt++;
	clrscr();

	if (usart_chrready()) {
		handle_uart();
	}

	ds3231_gettime(&cSt.time, 1);
	ds3231_getdate(&cSt.date, 1);

	switch (cSt.mode) {
		case 0:
			{
				showclock();
				break;
			}
		case 1:
			{
				showseconds();
				break;
			}
		case 2:
			{
				showwork();
				break;
			}
		case 3:
			{
				showrest();
				break;
			}
		case 4:
			{
				showdate();
				break;
			}
		case 5:
			{
				showyear();
				break;
			}
		default:
			break;
	}

	switch (cSt.bkmode) {
		case 0:
			{
				if ((tt&15) == 1) {
					rainbowcycling();
				}
				break;
			}
		case 1:
			{
				setbkcolor(0);
				break;
			}
		case 2:
			{
				setbkcolor(214);
				break;
			}
		case 3:
			{
				setbkcolor(86);
				break;
			}
		default:
			break;
	}

	refresh();

}

int main(void)
{
	init();
	while (1) {
		loop();
	}
	return 0;
}

