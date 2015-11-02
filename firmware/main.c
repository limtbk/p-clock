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

#define LED_CTRL0 PORT_D5
#define LED_CTRL1 PORT_D6
#define LED_CTRL2 PORT_D7
#define LED_CTRL3 PORT_B0
#define LED_CTRL4 PORT_B1
#define LED_CTRL5 PORT_B2

#define SND PORT_D2

#define DS3231_IN32KHZ PORT_C3

#define HC4052_A PORT_C2
#define HC4052_B PORT_C1
#define HC4052_INH PORT_C0

//TODO: move 2812 interaction to separate module
void refresh() { //TODO make all loop at asm, optimize nop with jp
	cli();
	for (uint8_t i = 0; i<5*3*3; i++) {
//		uint8_t ibyte = current[i];
//		uint8_t bc = 8;
//		uint8_t ibyte = 0x05;
		uint8_t ibyte = i;
		uint8_t bc = 8;
		asm volatile (
				"	ldi		%[bitcnt],8"			"\n\t"
				"btloop%=:"							"\n\t"
				"	sbi		%[port],%[nn]"			"\n\t" //PORT=1
				"	nop"							"\n\t"
				"	rol		%[byte]"				"\n\t"
				"	brcs	bit1%="					"\n\t"
				"	cbi		%[port],%[nn]"			"\n\t" //PORT=0
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	rjmp	bit0%="					"\n\t"
				"bit1%=:"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"bit0%=:"							"\n\t"
				"	cbi		%[port],%[nn]"			"\n\t" //PORT=0
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	subi	%[bitcnt],1"			"\n\t"
				"	brne	btloop%="				"\n\t"
				::
				[port] "I" (_SFR_IO_ADDR(PORT(LED_CTRL0))),
				[nn] "I" (PORTN(LED_CTRL0)),
				[bitcnt] "d" (bc),
				[byte] "r" (ibyte)

		);
	}
	for (uint8_t i = 0; i<5*3*3; i++) {
//		uint8_t ibyte = current[i];
//		uint8_t bc = 8;
//		uint8_t ibyte = 0x05;
		uint8_t ibyte = i;
		uint8_t bc = 8;
		asm volatile (
				"	ldi		%[bitcnt],8"			"\n\t"
				"btloop%=:"							"\n\t"
				"	sbi		%[port],%[nn]"			"\n\t" //PORT=1
				"	nop"							"\n\t"
				"	rol		%[byte]"				"\n\t"
				"	brcs	bit1%="					"\n\t"
				"	cbi		%[port],%[nn]"			"\n\t" //PORT=0
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	rjmp	bit0%="					"\n\t"
				"bit1%=:"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"bit0%=:"							"\n\t"
				"	cbi		%[port],%[nn]"			"\n\t" //PORT=0
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	subi	%[bitcnt],1"			"\n\t"
				"	brne	btloop%="				"\n\t"
				::
				[port] "I" (_SFR_IO_ADDR(PORT(LED_CTRL1))),
				[nn] "I" (PORTN(LED_CTRL1)),
				[bitcnt] "d" (bc),
				[byte] "r" (ibyte)

		);
	}
	for (uint8_t i = 0; i<5*3*3; i++) {
//		uint8_t ibyte = current[i];
//		uint8_t bc = 8;
//		uint8_t ibyte = 0x05;
		uint8_t ibyte = i;
		uint8_t bc = 8;
		asm volatile (
				"	ldi		%[bitcnt],8"			"\n\t"
				"btloop%=:"							"\n\t"
				"	sbi		%[port],%[nn]"			"\n\t" //PORT=1
				"	nop"							"\n\t"
				"	rol		%[byte]"				"\n\t"
				"	brcs	bit1%="					"\n\t"
				"	cbi		%[port],%[nn]"			"\n\t" //PORT=0
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	rjmp	bit0%="					"\n\t"
				"bit1%=:"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"bit0%=:"							"\n\t"
				"	cbi		%[port],%[nn]"			"\n\t" //PORT=0
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	nop"							"\n\t"
				"	subi	%[bitcnt],1"			"\n\t"
				"	brne	btloop%="				"\n\t"
				::
				[port] "I" (_SFR_IO_ADDR(PORT(LED_CTRL2))),
				[nn] "I" (PORTN(LED_CTRL2)),
				[bitcnt] "d" (bc),
				[byte] "r" (ibyte)

		);
	}

	sei();
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
}

void loop() {
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

