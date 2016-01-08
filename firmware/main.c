/*
 * main.c
 *
 *  Created on: May 22, 2014
 *      Author: lim
 */

#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/wdt.h>
#include <stdio.h>
#include <stdlib.h>
#include "ports.h"
#include "macro.h"
#include "usart.h"
#include "i2c.h"
#include "ds3231.h"
#include "ws2812.h"
#include "timer1.h"

#define LED PORTA_D13
#define PRGM PORTA_D2
#define SND PORTA_D11

typedef struct ClockStatusStruct {
	uint8_t mode;
	uint8_t mmode;
	uint8_t bkmode;
	uint8_t cycling;
	uint8_t sndon;
	TTime time;
	TDate date;
	TTime workTimeStart;
	TTime restTimeStart;
	uint8_t workTimeMinutes;
	uint8_t restTimeMinutes;
	uint8_t fps;
} TClockStatus;

TClockStatus cSt;

void clrscr() {
	for (uint8_t i = 0; i<P_TOTAL*3; i++) {
		new_pixels[i] = 0;
		current_pixels[i] = 0;
	}
}

void init() {
//	wdt_enable(WDTO_1S);
	usart_init();
	i2c_init();
	timer1_init();
	SETD(LED_CTRL);
	CLRP(LED_CTRL);

	SETD(LED);

	SETD(SND);
	cSt.sndon = 1;

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
	cSt.cycling = 1;

	usart_printstr("\n\rp-clock\n\r");
}

void playsnd() {
	//4kHz = 250us
	if (cSt.sndon) {
		for (uint16_t i = 0; i < 500; i++) {
			INVP(SND);
			timer1_delay_us2(250);
		}
	}
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
						"thhmmss - set time\n\r"
						"dyymmddw - set date\n\r"
						"wmm - set worktime\n\r"
						"rmm - set resttime\n\r"
						"mx - mode\n\r"
						"kx - color mode\n\r"
						"s - start work\n\r"
						"a - start rest\n\r"
						"p - play sound\n\r"
						"o - sound on/off\n\r"
						);
				break;
			}
		case 'p':
			{
				playsnd();
				break;
			}
		case 'o':
			{
				cSt.sndon = !cSt.sndon;
				usart_printstr("sound ");
				if (cSt.sndon) {
					usart_printstr("on");
				} else {
					usart_printstr("off");
				}
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
				uint8_t dow = usart_hextochar(usart_getchr());
				TDate date = tdate(dow, day, month, year);
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
				usart_printhex(cSt.bkmode);
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
		case 'f':
			{
				usart_printhex(cSt.fps);
				usart_printstr("\n\r");
#ifdef I2C_DEBUG
				uint16_t* dbg = dbginfo();
				for (uint8_t i = 0; i < 6; ++i) {
					usart_printhex(HI(dbg[i]));
					usart_printhex(LO(dbg[i]));
					usart_printstr(" ");
				}
				usart_printstr("\n\r");
#endif
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
	if (tt > P_TOTAL) {
		tt -= P_TOTAL;
	}
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
	uint8_t ss = sec % 2;
	current_pixels[(2*13+ss)*3+0] = pattern[(2*13+ss)*3+0];
	current_pixels[(2*13+ss)*3+1] = pattern[(2*13+ss)*3+1];
	current_pixels[(2*13+ss)*3+2] = pattern[(2*13+ss)*3+2];
}

void showseconds() {
	setnum(0, cSt.time.sec & 0x0F);
	setnum(1, (cSt.time.sec & 0xF0) >> 4);
	setnum(2, cSt.time.min & 0x0F);
	setnum(3, (cSt.time.min & 0xF0) >> 4);
}

void showdate() {
	setnum(0, cSt.date.month & 0x0F);
	setnum(1, (cSt.date.month & 0xF0) >> 4);
	setnum(2, cSt.date.day & 0x0F);
	setnum(3, (cSt.date.day & 0xF0) >> 4);
}

//void showdow() {
//	switch (cSt.date.weekday) {
//		case 1:
//			setchr(9, 21); //PN
//			setchr(5, 22);
//			break;
//		case 2:
//			setchr(9, 11); //VT
//			setchr(5, 23);
//			break;
//		case 3:
//			setchr(9, 12); //SR
//			setchr(5, 24);
//			break;
//		case 4:
//			setchr(9, 4); //4T
//			setchr(5, 23);
//			break;
//		case 5:
//			setchr(9, 21); //PT
//			setchr(5, 23);
//			break;
//		case 6:
//			setchr(9, 12); //SB
//			setchr(5, 25);
//			break;
//		case 7:
//			setchr(9, 11); //VS
//			setchr(5, 12);
//			break;
//		default:
//			break;
//	}
//}

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
		playsnd();
		startrest();
	} else {
		TTime timer;
		secondstotime(wtime-rtime, &timer);
		if (timer.hour) {
			setnum(0, timer.min & 0x0F);
			setnum(1, (timer.min & 0xF0) >> 4);
			setnum(2, timer.hour & 0x0F);
			setnum(3, (timer.hour & 0xF0) >> 4);
		} else {
			setnum(0, timer.sec & 0x0F);
			setnum(1, (timer.sec & 0xF0) >> 4);
			setnum(2, timer.min & 0x0F);
			setnum(3, (timer.min & 0xF0) >> 4);
		}
	}
}

void showrest() {
	int16_t ttime = timetoseconds(cSt.time);
	int16_t dtime = timetoseconds(cSt.restTimeStart);
	int16_t wtime = cSt.restTimeMinutes*60;

	int16_t rtime = ttime - dtime;
	if ((rtime<0) || (rtime>wtime)) {
		playsnd();
		startwork();
	} else {
		TTime timer;
		secondstotime(wtime-rtime, &timer);
		if (timer.hour) {
			setnum(0, timer.min & 0x0F);
			setnum(1, (timer.min & 0xF0) >> 4);
			setnum(2, timer.hour & 0x0F);
			setnum(3, (timer.hour & 0xF0) >> 4);
		} else {
			setnum(0, timer.sec & 0x0F);
			setnum(1, (timer.sec & 0xF0) >> 4);
			setnum(2, timer.min & 0x0F);
			setnum(3, (timer.min & 0xF0) >> 4);
		}

	}
}

void loop() {
	static TTime prevTime;
	static uint8_t fps = 0;
	fps++;
//	wdt_reset();

	static uint8_t tt = 0;
	tt++;
//	usart_printhex(tt);
//	usart_printstr("\n\r");
	clrscr();

	if (usart_chrready()) {
		handle_uart();
	}

	ds3231_gettime(&cSt.time, 1);
//	uint16_t* dbg = dbginfo();
//	for (uint8_t i = 0; i < 6; ++i) {
//		usart_printhex(HI(dbg[i]));
//		usart_printhex(LO(dbg[i]));
//		usart_printstr(" ");
//	}
//	usart_printstr("\n\r");
	if (cSt.time.sec != prevTime.sec) {
		if ((cSt.time.min == 0) && (cSt.time.sec == 0)) {
			playsnd();
		}
		ds3231_getdate(&cSt.date, 1);
//		dbg = dbginfo();
//		for (uint8_t i = 0; i < 6; ++i) {
//			usart_printhex(HI(dbg[i]));
//			usart_printhex(LO(dbg[i]));
//			usart_printstr(" ");
//		}
//		usart_printstr("\n\r");
		prevTime = cSt.time;
		cSt.fps = fps;
		fps = 0;
	}

	cSt.mmode = cSt.mode;

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
				setbkcolor(0); //red
				break;
			}
		case 2:
			{
				setbkcolor(172); //blue
				break;
			}
		case 3:
			{
				setbkcolor(86); //green
				break;
			}
		case 4:
			{
				setbkcolor(129); //green+blue
				break;
			}
		case 5:
			{
				setbkcolor(43); //red+green
				break;
			}
		case 6:
			{
				setbkcolor(214); //red+blue
				break;
			}
		case 7:
			{
				setbkcolor(240);
				break;
			}
		case 8:
			{
				setbkcolor(110);
				break;
			}
		default:
			break;
	}

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

