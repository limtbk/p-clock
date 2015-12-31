/*
 * ws2812.c
 *
 *  Created on: Nov 4, 2015
 *      Author: lim
 */

#include "ws2812.h"
#include <avr/pgmspace.h>

#define send_bytesm(port__, ddr__, pin__, portnum__, source__) \
			uint8_t __bitcnt__ = 8;\
			asm volatile (\
				"	ldi		%[bitcnt],8"			"\n\t"\
				"btloop%=:"							"\n\t"\
				"	sbi		%[port],%[nn]"			"\n\t"\
				"	nop"							"\n\t"\
				"	rol		%[byte]"				"\n\t"\
				"	brcs	bit1%="					"\n\t"\
				"	cbi		%[port],%[nn]"			"\n\t"\
				"	nop"							"\n\t"\
				"	nop"							"\n\t"\
				"	rjmp	bit0%="					"\n\t"\
				"bit1%=:"							"\n\t"\
				"	nop"							"\n\t"\
				"	nop"							"\n\t"\
				"	nop"							"\n\t"\
				"	nop"							"\n\t"\
				"	nop"							"\n\t"\
				"bit0%=:"							"\n\t"\
				"	cbi		%[port],%[nn]"			"\n\t"\
				"	nop"							"\n\t"\
				"	nop"							"\n\t"\
				"	nop"							"\n\t"\
				"	nop"							"\n\t"\
				"	subi	%[bitcnt],1"			"\n\t"\
				"	brne	btloop%="				"\n\t"\
				::\
				[port] "I" (_SFR_IO_ADDR(port__)),\
				[nn] "I" (portnum__),\
				[bitcnt] "d" (__bitcnt__),\
				[byte] "r" (source__)\
			)

#define send_bytes(port, source) send_bytesm(port, source)

const uint16_t PROGMEM numfnt [] = {
	0b0111010101110000, //0
	0b0000010011111000, //1
	0b1100111110010000, //2
	0b1000111101011000, //3
	0b0011101011111000, //4
	0b1011111101001000, //5
	0b0111011101000000, //6
	0b1100111000011000, //7
	0b0101011101010000, //8
	0b0001011101110000, //9
	0b1111011011110000, //A
	0b1111111101010000, //B
	0b0111010101010000, //C
	0b1111110101110000, //D
	0b1111111110001000, //E
	0b1111111000001000  //F
};

void setnum(uint8_t apos, uint8_t num) {
	uint8_t pos = 3 - apos;
	uint8_t saddr = pos*(D_TOTAL);
	saddr += (pos>1)?2:0;
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

void refresh() {
	cli();
	for (uint8_t i = 0; i<P_TOTAL*3; i++) {
		uint8_t ibyte = current_pixels[i+0];
		send_bytes(LED_CTRL, ibyte);
	}
	sei();
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


