/*
 * main.c
 *
 *  Created on: May 22, 2014
 *      Author: lim
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include "ports.h"
#include "macro.h"
#include "usart.h"
#include "i2c.h"
#include "ds13xx.h"

#define LED PORTA_D13
#define PRGM PORTA_D2
#define LEDCTRL PORTA_D8

const uint16_t PROGMEM numfnt [] = {
		0b0111010101110000,
		0b0000010011111000,
		0b1100111110010000,
		0b1000111101011000,
		0b0011101011111000,
		0b1011111101001000,
		0b0111011101000000,
		0b1100111000011000,
		0b0101011101010000,
		0b0001011101110000,
		0b1111011011110000,
		0b1111111101010000,
		0b0111010101010000,
		0b1111110101110000,
		0b1111111110001000,
		0b1111111000001000

//		0b1111110111111000,
//		0b0000000011111000,
//		0b1110111110111000,
//		0b1000111111111000,
//		0b0011101011111000,
//		0b1011111111101000,
//		0b1111111111101000,
//		0b0000110011111000,
//		0b1111111111111000,
//		0b1011111111111000,
//		0b1111011011110000,
//		0b1111111101010000,
//		0b0111010101010000,
//		0b1111110101110000,
//		0b1111111110001000,
//		0b1111111000001000
};

uint8_t j = 0;
uint16_t k = 0;
uint8_t pixels[54*3];
uint8_t current[54*3];
uint8_t needrefresh = 1;
uint8_t needshow = 1;
uint8_t finished = 0;

uint8_t hours = 0;
uint8_t minutes = 0;
uint8_t seconds = 0;
uint8_t t_minutes = 0;
uint8_t t_seconds = 0;
uint8_t t_workmin = 45;
uint8_t t_restmin = 5;

uint8_t defred = 16;
uint8_t defgreen = 4;
uint8_t defblue = 0;
uint8_t timnred = 16;
uint8_t timngreen = 4;
uint8_t timnblue = 0;
uint8_t timwred = 32;
uint8_t timwgreen = 0;
uint8_t timwblue = 0;
uint8_t timrred = 0;
uint8_t timrgreen = 0;
uint8_t timrblue = 16;

uint16_t thr = 0;
uint16_t thrmax = 3200;

uint8_t mode = 0; //0 - hh:mm, 1 - mm:ss, 2 - ptimer wrk, 3 - ptimer rest

void clrscr() {
	for (uint8_t i = 0; i<54*3; i++) {
		pixels[i] = 0;
		current[i] = 0;
	}
}

void setnum(uint8_t pos, uint8_t num) {
	uint8_t saddr = pos*13;
	saddr += (pos>1)?2:0;
	uint16_t symbit = pgm_read_word(&numfnt[num]);
	uint8_t i = 13;
	while (i) {
		uint8_t msk = (symbit&0x8000)?255:0;
		uint8_t pxr = defred&msk;
		uint8_t pxg = defgreen&msk;
		uint8_t pxb = defblue&msk;
		pixels[saddr*3+0] = pxg&msk;
		pixels[saddr*3+1] = pxr&msk;
		pixels[saddr*3+2] = pxb&msk;
		symbit = symbit<<1;
		saddr++;
		i--;
	}
}

void setpt(uint8_t pos, uint8_t num) {
	uint8_t saddr = 2*13+(pos&1);
	uint8_t pxr = defred;
	uint8_t pxg = defgreen;
	uint8_t pxb = defblue;
	uint8_t msk = (num&1)?255:0;
	pixels[saddr*3+0] = pxg&msk;
	pixels[saddr*3+1] = pxr&msk;
	pixels[saddr*3+2] = pxb&msk;
}

void gotocurrent() {
	for (uint8_t i = 0; i<54*3; i++) {
		if (current[i]<pixels[i]) {
			current[i] = current[i] + 1;
			finished = 0;
		} else if (current[i]>pixels[i]) {
			current[i] = current[i] - 1;
			finished = 0;
		}
	}
}

//TODO: move 2812 interaction to separate module
void refresh() { //TODO make all loop at asm, optimize nop with jp
	cli();
	for (uint8_t i = 0; i<54*3; i++) {
		uint8_t ibyte = current[i];
		uint8_t bc = 8;
		asm volatile (
//				"	cli"							"\n\t"
				"	eor		__tmp_reg__,__tmp_reg__"	"\n\t"
				"	inc		__tmp_reg__"			"\n\t"
				"	ldi		%[bitcnt],8"			"\n\t"
				"btloop:"							"\n\t"
				"	out		%[port],__tmp_reg__"	"\n\t"
				"	rol		%[byte]"				"\n\t"
				"	nop"							"\n\t"
				"	brcs	bit1"					"\n\t"
				"	out		%[port],__zero_reg__"	"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	rjmp	bit0"					"\n\t"
				"bit1:"								"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"bit0:"								"\n\t"
				"	out		%[port],__zero_reg__"	"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	subi	%[bitcnt],1"			"\n\t"
				"	brne	btloop"					"\n\t"
//				"	sei"							"\n\t"
				::
				  [bitcnt] "d" (bc),
				  [port] "I" (_SFR_IO_ADDR(PORTB)),
				  [byte] "r" (ibyte)
		);
	}
	sei();
}

void startwork() {
	t_minutes = t_workmin;
	t_seconds = 0;
	mode = 2;
	usart_printstr("Start work\n\r\0");
}

void startrest() {
	t_minutes = t_restmin;
	t_seconds = 0;
	mode = 3;
	usart_printstr("Start rest\n\r\0");
}

void timeplus() {
	seconds += 1;
	while (seconds>59) {
		seconds -= 60;
		minutes += 1;
		while (minutes>59) {
			minutes -= 60;
			hours += 1;
			while (hours>23) {
				hours -= 24;
			}
		}
	}

	if ((t_minutes>0) || (t_seconds>0)) {
		t_seconds -= 1;
		if (t_seconds>59) {
			t_seconds = 59;
			t_minutes -= 1;
		}
	} else {
		if (mode==2) {
			startrest();
		} else if (mode==3) {
			startwork();
		}
	}
}

void showtime() {
	switch (mode) {
		case 1: {
			defred = timnred;
			defgreen = timngreen;
			defblue = timnblue;

			uint8_t sec10 = 0;
			uint8_t sec1 = seconds;
			while (sec1>9) {
				sec1 -= 10;
				sec10 += 1;
			}
			uint8_t min10 = 0;
			uint8_t min1 = minutes;
			while (min1>9) {
				min1 -= 10;
				min10 += 1;
			}
			setnum(0,min10);
			setnum(1,min1);
			setnum(2,sec10);
			setnum(3,sec1);
			setpt(0,0);
			setpt(1,0);
			break;
		}
		case 2: {
			defred = timwred;
			defgreen = timwgreen;
			defblue = timwblue;

			uint8_t sec10 = 0;
			uint8_t sec1 = t_seconds;
			while (sec1>9) {
				sec1 -= 10;
				sec10 += 1;
			}
			uint8_t min10 = 0;
			uint8_t min1 = t_minutes;
			while (min1>9) {
				min1 -= 10;
				min10 += 1;
			}
			setnum(0,min10);
			setnum(1,min1);
			setnum(2,sec10);
			setnum(3,sec1);
			setpt(0,1);
			setpt(1,0);
			break;
		}
		case 3: {
			defred = timrred;
			defgreen = timrgreen;
			defblue = timrblue;

			uint8_t sec10 = 0;
			uint8_t sec1 = t_seconds;
			while (sec1>9) {
				sec1 -= 10;
				sec10 += 1;
			}
			uint8_t min10 = 0;
			uint8_t min1 = t_minutes;
			while (min1>9) {
				min1 -= 10;
				min10 += 1;
			}
			setnum(0,min10);
			setnum(1,min1);
			setnum(2,sec10);
			setnum(3,sec1);
			setpt(0,0);
			setpt(1,1);
			break;
		}
		case 0:
		default:
		{
			defred = timnred;
			defgreen = timngreen;
			defblue = timnblue;

			uint8_t min10 = 0;
			uint8_t min1 = minutes;
			while (min1>9) {
				min1 -= 10;
				min10 += 1;
			}
			uint8_t hr10 = 0;
			uint8_t hr1 = hours;
			while (hr1>9) {
				hr1 -= 10;
				hr10 += 1;
			}
			setnum(0,hr10);
			setnum(1,hr1);
			setnum(2,min10);
			setnum(3,min1);
			setpt(0,seconds&1);
			setpt(1,(~seconds)&1);

			usart_printhex(hours);
			usart_printhex(minutes);
			usart_printhex(seconds);
			usart_printstr("\n\r");
			break;
		}
	}

	needrefresh = 1;
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
//	cli();
	timeplus();
	needshow = 1;
//	sei();
}


void init() {
	usart_init();
	i2c_init();
	initTimer();
	SETD(LED);
	SETD(LEDCTRL);
	//SETP(LED);
	CLRD(PRGM);
	SETP(PRGM);
	j = 0;
	k = 0;
	needrefresh = 1;
	needshow = 1;
	clrscr();
	thrmax = 3200;
//	setnum(0,2);
//	setnum(1,3);
//	setnum(2,2);
//	setnum(3,7);
	usart_printstr("Pomodoro clock v0.2\n\r\0");

	TTime time;
	ds13xx_gettime(&time, 0);
	hours = time.hour;
	minutes = time.min;
	seconds = time.sec;
}

void chkpgm() {
	if (!PINV(PRGM)) {
		void *bl = (void *)0x3c00;
		goto *bl;
	}
}

void loop() {
	chkpgm();
	if (needrefresh) {
		thr++;
		if (thr > thrmax) { //TODO: throttle with timer
			thr = 0;
			finished = 1;
			gotocurrent();
			refresh();
			if (finished) {
				needrefresh = 0;
			}
		}
	}
	if (needshow) {
		showtime();
		needshow = 0;
	}
	if (usart_chrready()) {
		char ch = usart_getchr();
//		usart_putchr(ch);
		switch (ch) { //TODO: add help
			case 'h':
			{
				usart_printstr("\n\r"
						"h - help\n\r"
						"txxxxxx - set time\n\r"
						"wxx - set worktime\n\r"
						"rxx - set resttime\n\r"
						"mx - mode\n\r"
						"s - start work\n\r"
						"a - start rest\n\r"
						"fxxxx - set fade delay\n\r"
						"cyxxxxxx - set color for k\n\r");
				break;
			}
			case 't':
			{
				usart_printstr("time");
				hours = usart_hextochar(usart_getchr())*10;
				hours += usart_hextochar(usart_getchr());
				usart_printhex(hours);
				minutes = usart_hextochar(usart_getchr())*10;
				minutes += usart_hextochar(usart_getchr());
				usart_printhex(minutes);
				seconds = usart_hextochar(usart_getchr())*10;
				seconds += usart_hextochar(usart_getchr());
				usart_printhex(seconds);
				TTime time;
				time.hour = hours;
				time.min = minutes;
				time.sec = seconds;
				ds13xx_settime(time, 0);
				break;
			}
			case 'w':
			{
				usart_printstr("worktime");
				t_workmin = usart_hextochar(usart_getchr())*10;
				t_workmin += usart_hextochar(usart_getchr());
				usart_printhex(t_workmin);
				break;
			}
			case 'r':
			{
				usart_printstr("resttime");
				t_restmin = usart_hextochar(usart_getchr())*10;
				t_restmin += usart_hextochar(usart_getchr());
				usart_printhex(t_restmin);
				break;
			}
			case 'm':
			{
				usart_printstr("mode");
				mode = usart_hextochar(usart_getchr());
				usart_printhex(mode);
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
			case 'f':
			{
				usart_printstr("fade");
				thrmax = usart_hextochar(usart_getchr())*4096;
				thrmax += usart_hextochar(usart_getchr())*256;
				thrmax += usart_hextochar(usart_getchr())*16;
				thrmax += usart_hextochar(usart_getchr());
				usart_printhex((thrmax&0xff00)>>8);
				usart_printhex(thrmax&0x00ff);
				break;
			}
			case 'c':
			{
				usart_printstr("color");
				uint8_t colnum = usart_hextochar(usart_getchr());
				usart_printhex(colnum);
				uint8_t rcol = usart_hextochar(usart_getchr())*16;
				rcol += usart_hextochar(usart_getchr());
				usart_printhex(rcol);
				uint8_t gcol = usart_hextochar(usart_getchr())*16;
				gcol += usart_hextochar(usart_getchr());
				usart_printhex(gcol);
				uint8_t bcol = usart_hextochar(usart_getchr())*16;
				bcol += usart_hextochar(usart_getchr());
				usart_printhex(bcol);

			switch (colnum) {
				case 0:
					timnred = rcol;
					timngreen = gcol;
					timnblue = bcol;
					break;
				case 1:
					timwred = rcol;
					timwgreen = gcol;
					timwblue = bcol;
					break;
				case 2:
					timrred = rcol;
					timrgreen = gcol;
					timrblue = bcol;
					break;
				default:
					break;
			}
			}
			default:
				break;
		}
		usart_printstr("\n\rok\n\r");

	}


}

int main(void)
{
	init();
	while (1) {
		loop();
	}
	return 0;
}

