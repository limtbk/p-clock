/*
 * ws2812.c
 *
 *  Created on: Nov 4, 2015
 *      Author: lim
 */

#include "ws2812.h"
#include <avr/pgmspace.h>

const uint16_t PROGMEM numfnt [] = {
	0b0111010001011100,
	0b1000011111100100,
	0b1001010101110010,
	0b0101110101100010,
	0b1111100100001110,
	0b0100110101101110,
	0b0100010101011100,
	0b0001111101000010,
	0b0101010101010100,
	0b0111010101000100,
	0b1111000101111100,
	0b0101010101111110,
	0b1000110001011100,
	0b0111010001111110,
	0b0111010001111110,
	0b1000110101111110,
	0b0000100101111110
};

void setnum(uint8_t pos, uint8_t num) {
	uint8_t saddr = pos*(D_TOTAL+D_HEIGHT);
	saddr += (pos>1)?D_HEIGHT*2:0;
	uint16_t symbit = pgm_read_word(&numfnt[num]);
	uint8_t i = D_TOTAL;
	while (i) {
		uint8_t msk = (symbit&0x8000)?255:0;
		current_pixels[saddr*3+0] = pattern[saddr*3+0]&msk;
		current_pixels[saddr*3+1] = pattern[saddr*3+1]&msk;
		current_pixels[saddr*3+2] = pattern[saddr*3+2]&msk;
		symbit = symbit<<1;
		saddr++;
		i--;
	}
}

void refresh() { //TODO make all loop at asm, optimize nop with jp
	cli();
	for (uint8_t i = 0; i<5*3*3; i++) {
		uint8_t ibyte = current_pixels[i+0];
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
		uint8_t ibyte = current_pixels[i+15*3];
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
		uint8_t ibyte = current_pixels[i+30*3];
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
	for (uint8_t i = 0; i<5*3*3; i++) {
		uint8_t ibyte = current_pixels[i+45*3];
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
				[port] "I" (_SFR_IO_ADDR(PORT(LED_CTRL3))),
				[nn] "I" (PORTN(LED_CTRL3)),
				[bitcnt] "d" (bc),
				[byte] "r" (ibyte)

		);
	}
	for (uint8_t i = 0; i<5*3*3; i++) {
		uint8_t ibyte = current_pixels[i+60*3];
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
				[port] "I" (_SFR_IO_ADDR(PORT(LED_CTRL4))),
				[nn] "I" (PORTN(LED_CTRL4)),
				[bitcnt] "d" (bc),
				[byte] "r" (ibyte)

		);
	}
	for (uint8_t i = 0; i<5*3*3; i++) {
		uint8_t ibyte = current_pixels[i+75*3];
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
				[port] "I" (_SFR_IO_ADDR(PORT(LED_CTRL5))),
				[nn] "I" (PORTN(LED_CTRL5)),
				[bitcnt] "d" (bc),
				[byte] "r" (ibyte)

		);
	}
	sei();
}

