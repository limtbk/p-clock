/*
 * main.c
 *
 *  Created on: May 22, 2014
 *      Author: lim
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <stdlib.h>
#include "ports.h"
#include "macro.h"
#include "usart.h"
#include "i2c.h"
#include "ds3231.h"
#include "ws2812.h"
#include "timer1.h"
#include "bmp085.h"
#include "dht11.h"

#define SND PORT_D2

#define DS3231_IN32KHZ PORT_C3 //PCINT11 on PCIE1
#define DS3231_INSQW PORT_D4 //PCINT20 on PCIE2
#define DS3231_RST PORT_D3

typedef struct AvgInt8Struct {
	int16_t value;
	int8_t measures;
	int8_t hour;
	int8_t values[24];
} AvgInt8;

typedef struct AvgInt16Struct {
	int32_t value;
	int8_t measures;
	int8_t hour;
	int16_t values[24];
} AvgInt16;

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
	TTemperature temp0;
	int8_t temp1;
	int8_t temp2;
	int16_t pressure;
	int8_t humidity;
	AvgInt8 avgHummidity;
	AvgInt8 avgTemperature;
	AvgInt16 avgPressure;
	int8_t timezone;
	uint8_t dst;
	uint8_t dst_on_month;
	uint8_t dst_on_day;
	uint8_t dst_off_month;
	uint8_t dst_off_day;
} TClockStatus;

TClockStatus cSt;

void addAvg8(AvgInt8 *avg, int8_t val) {
	uint8_t hour = dectobin(cSt.time.hour);
	if (hour==avg->hour) {
		avg->value += val;
		avg->measures++;
	} else {
		avg->values[avg->hour] = (avg->value / avg->measures);
		avg->hour = hour;
		avg->measures = 1;
		avg->value = val;
	}
}

void addAvg16(AvgInt16 *avg, int16_t val) {
	uint8_t hour = dectobin(cSt.time.hour);
	if (hour==avg->hour) {
		avg->value += val;
		avg->measures++;
	} else {
		avg->values[avg->hour] = (avg->value / avg->measures);
		avg->hour = hour;
		avg->measures = 1;
		avg->value = val;
	}
}

void reboot() {
#ifdef __AVR_ATmega328P__
	void *bl = (void *)0x3c00; //for ATMega328 with bootloader
	goto *bl; //Yep, its hack to reboot without watchdog
#endif
}

void clrscr() {
	for (uint8_t i = 0; i<P_TOTAL*3; i++) {
		new_pixels[i] = 0;
		current_pixels[i] = 0;
	}
}

//ISR(PCINT0_vect) {
//	static uint16_t t = 0;
//	t++;
//	if (t == 0) {
////		usart_putchr('0');
//	}
//}
//
//ISR(PCINT1_vect) {
//	static uint32_t t = 0;
//	static uint8_t s = 0;
//
//	t++;
//	if (s != cSt.time.sec) {
//		s = cSt.time.sec;
////		usart_printhex(t>>24);
////		usart_printhex(t>>16);
////		usart_printhex(t>>8);
////		usart_printhex(t);
////		usart_printstr("\n\r");
//		t = 0;
//	}
////	if (t == 32768) {
////		t = 0;
//////		usart_putchr('1');
////	}
//}
//
//ISR(PCINT2_vect) {
//	static uint16_t t = 0;
//	t++;
//	if (t == 0) {
////		usart_putchr('2');
//	}
//}
void loadsettings() {
	cSt.timezone = eeprom_read_byte((void *)2);
	cSt.dst = eeprom_read_byte((void *)3);
	cSt.dst_on_month = eeprom_read_byte((void *)4);
	cSt.dst_on_day = eeprom_read_byte((void *)5);
	cSt.dst_off_month = eeprom_read_byte((void *)6);
	cSt.dst_off_day = eeprom_read_byte((void *)7);
}

void savesettings() {
	eeprom_write_byte((void *)2, cSt.timezone);
	eeprom_busy_wait();
	eeprom_write_byte((void *)3, cSt.dst);
	eeprom_busy_wait();
	eeprom_write_byte((void *)4, cSt.dst_on_month);
	eeprom_busy_wait();
	eeprom_write_byte((void *)5, cSt.dst_on_day);
	eeprom_busy_wait();
	eeprom_write_byte((void *)6, cSt.dst_off_month);
	eeprom_busy_wait();
	eeprom_write_byte((void *)7, cSt.dst_off_day);
	eeprom_busy_wait();
}

void init() {
	uint8_t reboots = eeprom_read_byte(0);
	if (reboots) {
		eeprom_write_byte(0, reboots-1);
		eeprom_busy_wait();
		reboot();
	}
	loadsettings();

	usart_init();
	i2c_init();
	timer1_init();
	bmp085_init();
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

	SETP(DS3231_IN32KHZ); //set pull-up resistor
	CLRD(DS3231_IN32KHZ);
	SETP(DS3231_INSQW); //set pull-up resistor
	CLRD(DS3231_INSQW);
	SETP(DS3231_RST);
	SETD(DS3231_RST);

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
		for (uint16_t i = 0; i < 1000; i++) {
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
						"thhmmss - set GMT time\n\r"
						"dyymmddw - set GMT date\n\r"
						"wmm - set worktime\n\r"
						"rmm - set resttime\n\r"
						"mx - mode\n\r"
						"kx - color mode\n\r"
						"s - start work\n\r"
						"a - start rest\n\r"
						"c - cycling on/off\n\r"
						"q - read last 24hr stats\n\r"
						"p - play sound\n\r"
						"o - sound on/off\n\r"
						"D - DST\n\r"
						"T+hh - timezone\n\r"
						"Ymmdd - DST on date (+1hr to timezone)\n\r"
						"Ummdd - DST off date\n\r"
						"R - reboot\n\r"
						"F - wait for new firmware during 20 reboots\n\r"
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
		case 'c':
			{
				cSt.cycling = !cSt.cycling;
				usart_printstr("cycling ");
				if (cSt.cycling) {
					usart_printstr("on");
				} else {
					usart_printstr("off");
				}
				break;
			}
		case 'D':
			{
				cSt.dst = !cSt.dst;
				usart_printstr("daylight saving time ");
				if (cSt.dst) {
					usart_printstr("on");
				} else {
					usart_printstr("off");
				}
				savesettings();
				break;
			}
		case 'T':
			{
				usart_printstr("timezone");
				char s = usart_getchr();
				if ((s == '+') || s == '-') {
					usart_putchr(s);
					cSt.timezone = usart_hextochar(usart_getchr())*10;
					cSt.timezone += usart_hextochar(usart_getchr());
				} else {
					cSt.timezone = usart_hextochar(s)*10;
					cSt.timezone += usart_hextochar(usart_getchr());
				}
				if (s == '-') {
					cSt.timezone = -cSt.timezone;
				}
				usart_printhex(cSt.timezone);
				savesettings();
				break;
			}
		case 'q':
			{
				usart_printstr("\n\rtem ");
				uint8_t hour = dectobin(cSt.time.hour);
				for (uint8_t i = 0; i < 24; i++) {
					usart_printhex(cSt.avgTemperature.values[(hour+i+1)%24]);
				}
				usart_printstr("\n\rprs ");
				for (uint8_t i = 0; i < 24; i++) {
					usart_printhex(cSt.avgPressure.values[(hour+i+1)%24]/256);
					usart_printhex(cSt.avgPressure.values[(hour+i+1)%24]%256);
				}
				usart_printstr("\n\rhum ");
				for (uint8_t i = 0; i < 24; i++) {
					usart_printhex(cSt.avgHummidity.values[(hour+i+1)%24]);
				}

				break;
			}
		case 'Y':
			{
				usart_printstr("DST on date current ");
				usart_printhex(cSt.dst_on_month);
				usart_printhex(cSt.dst_on_day);
				usart_printstr(" new ");
				uint8_t month = usart_hextochar(usart_getchr())*10;
				month += usart_hextochar(usart_getchr());
				usart_printhex(month);
				uint8_t day = usart_hextochar(usart_getchr())*10;
				day += usart_hextochar(usart_getchr());
				usart_printhex(day);
				cSt.dst_on_day = day;
				cSt.dst_on_month = month;
				savesettings();
				break;
			}
		case 'U':
			{
				usart_printstr("DST off date current ");
				usart_printhex(cSt.dst_off_month);
				usart_printhex(cSt.dst_off_day);
				usart_printstr(" new ");
				uint8_t month = usart_hextochar(usart_getchr())*10;
				month += usart_hextochar(usart_getchr());
				usart_printhex(month);
				uint8_t day = usart_hextochar(usart_getchr())*10;
				day += usart_hextochar(usart_getchr());
				usart_printhex(day);
				cSt.dst_off_day = day;
				cSt.dst_off_month = month;
				savesettings();
				break;
			}
		case 'F':
			{
				eeprom_write_byte(0, 20);
				eeprom_busy_wait();
				break;
			}
		case 'R':
			{
				reboot();
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
	setnum(0, cSt.date.month & 0x0F);
	setnum(1, (cSt.date.month & 0xF0) >> 4);
	setnum(2, cSt.date.day & 0x0F);
	setnum(3, (cSt.date.day & 0xF0) >> 4);
}

void showdow() {
	switch (cSt.date.weekday) {
		case 1:
			setchr(9, 21); //PN
			setchr(5, 22);
			break;
		case 2:
			setchr(9, 11); //VT
			setchr(5, 23);
			break;
		case 3:
			setchr(9, 12); //SR
			setchr(5, 24);
			break;
		case 4:
			setchr(9, 4); //4T
			setchr(5, 23);
			break;
		case 5:
			setchr(9, 21); //PT
			setchr(5, 23);
			break;
		case 6:
			setchr(9, 12); //SB
			setchr(5, 25);
			break;
		case 7:
			setchr(9, 11); //VS
			setchr(5, 12);
			break;
		default:
			break;
	}
}

void showyear() {

	setchr(1, cSt.date.year & 0x0F);
	setchr(5, (cSt.date.year & 0xF0) >> 4);
	setchr(9, 0);
	setchr(13, 2);
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

void showtemp0() {
	setchr(5, 20);
	setchr(1, 12);
	setchr(9, cSt.temp0.intPart & 0x0F);
	setchr(13, (cSt.temp0.intPart & 0xF0) >> 4);
}

void showtemp1() {
	setchr(5, 20);
	setchr(1, 12);
	setchr(9, cSt.temp1 & 0x0F);
	setchr(13, (cSt.temp1 & 0xF0) >> 4);
}

void showtemp2() {
	setchr(5, 20);
	setchr(1, 12);
	setchr(9, cSt.temp2 & 0x0F);
	setchr(13, (cSt.temp2 & 0xF0) >> 4);
}

void showpressure() {
	setchr(1, 18);
	setchr(5, cSt.pressure%10);
	setchr(9, (cSt.pressure/10)%10);
	setchr(13, (cSt.pressure/100)%10);
}

void showhumidity() {
	if ((cSt.humidity/100)%10) {
		setchr(1, 19);
		setchr(5, cSt.humidity%10);
		setchr(9, (cSt.humidity/10)%10);
		setchr(13, (cSt.humidity/100)%10);
	} else {
		setchr(3, 19);
		setchr(7, cSt.humidity%10);
		setchr(11, (cSt.humidity/10)%10);
	}
}

void loop() {
	static uint8_t tt = 0;
	tt++;
	clrscr();

	if (usart_chrready()) {
		handle_uart();
	}

	uint32_t timestamp = ds3231_gettimestamp();
	timestamp += cSt.timezone * 3600;
	if (cSt.dst) {
		if ((cSt.date.month>cSt.dst_on_month) || ((cSt.date.month==cSt.dst_on_month) && (cSt.date.day>=cSt.dst_on_day))) {
			if ((cSt.date.month<cSt.dst_off_month) || ((cSt.date.month==cSt.dst_off_month) && (cSt.date.day<cSt.dst_off_day))) {
				timestamp += 3600;
			}
		}
	}

	timestamptodatetime(timestamp, &cSt.date, &cSt.time);

	if ((cSt.time.min == 0) && (cSt.time.sec == 0)) {
		playsnd();
		timer1_delay_ms(900);
	}

	if (cSt.mode == 6) {
		if (cSt.mmode != cSt.mode) {
			ds3231_gettemperature(&cSt.temp0, 1);
			cSt.temp0.intPart = cSt.temp0.intPart - 7;
			addAvg8(&cSt.avgTemperature, dectobin(cSt.temp0.intPart));
		}
	}
	if (cSt.mode == 7) {
		if (cSt.mmode != cSt.mode) {
			int8_t temp = bmp085_gettemperature();
			cSt.temp1 = bintodec(temp)-3;
			addAvg8(&cSt.avgTemperature, temp);
		}
	}
	if (cSt.mode == 8) {
		if (cSt.mmode != cSt.mode) {
			cSt.pressure = (bmp085_getpressure()*10)/1333; // Pa to mmHg
			addAvg16(&cSt.avgPressure, cSt.pressure);
		}
	}
	if (cSt.mode == 9) {
		if (cSt.mmode != cSt.mode) {
			cSt.humidity = dht11_gethumidity();
			addAvg8(&cSt.avgHummidity, cSt.humidity);
		}
	}
	if (cSt.mode == 10) {
		if (cSt.mmode != cSt.mode) {
			int8_t temp = dht11_gettemperature();
			cSt.temp2 = bintodec(temp)-2;
			addAvg8(&cSt.avgTemperature, temp);
		}
	}
	cSt.mmode = cSt.mode;

	if (cSt.cycling) {
		switch (dectobin(cSt.time.sec)/3) {
			case 6:
				{
					cSt.mode = 10;
					cSt.bkmode = 1;
					break;
				}
			case 8:
				{
					cSt.mode = 8;
					cSt.bkmode = 3;
					break;
				}
			case 10:
				{
					cSt.mode = 9;
					cSt.bkmode = 2;
					break;
				}
			case 14:
				{
					cSt.mode = 11;
					cSt.bkmode = 8;
					break;
				}
			case 15:
				{
					cSt.mode = 4;
					cSt.bkmode = 7;
					break;
				}
			case 16:
				{
					cSt.mode = 5;
					cSt.bkmode = 6;
					break;
				}
			default:
				{
					cSt.mode = 0;
					cSt.bkmode = 0;
					break;
				}
		}
	}

	switch (cSt.bkmode) {
		case 0:
			{
				if ((tt&7) == 1) {
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
		case 4:
			{
				setbkcolor(150);
				break;
			}
		case 5:
			{
				setbkcolor(43);
				break;
			}
		case 6:
			{
				setbkcolor(172);
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
        case 6:
            {
                showtemp0();
                break;
            }
        case 7:
            {
                showtemp1();
                break;
            }
        case 8:
            {
                showpressure();
                break;
            }
        case 9:
            {
                showhumidity();
                break;
            }
        case 10:
            {
                showtemp2();
                break;
            }
        case 11:
            {
                showdow();
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

