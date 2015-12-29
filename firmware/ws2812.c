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
	0b1111110001111110, //0
	0b1111100000000000, //1
	0b1011110101111010, //2
	0b1111110101101010, //3
	0b1111100100001110, //4
	0b1110110101101110, //5
	0b1110110101111110, //6
	0b1111100001000010, //7
	0b1111110101111110, //8
	0b1111110101101110, //9
	0b1111000101111100, //A
	0b0101010101111110, //B
	0b1000110001011100, //C
	0b0111010001111110, //D
	0b0111010001111110, //E
	0b1000110101111110, //F
	0b0000100101111110, //G
	0b1000000000000000, //dot
	0b1100011100111000, //m
	0b1001100100110010, //% 19
	0b0000000011000110, //gradus 20
	0b1111100001111110, //Pe 21
	0b1111100100111110, //Ne 22
	0b0000111111000010, //Te 23
	0b0001000101111110, //Re 24
	0b0100110101111110, //Be 25
	0b0111010001011100, //0
	0b1000011111100100, //1
	0b1001010101110010, //2
	0b0101110101100010, //3
	0b1111100100001110, //4
	0b0100110101101110, //5
	0b0100010101011100, //6
	0b0001111101000010, //7
	0b0101010101010100, //8
	0b0111010101000100, //9
	0b1111000101111100, //A
	0b0101010101111110, //B
	0b1000110001011100, //C
	0b0111010001111110, //D
	0b0111010001111110, //E
	0b1000110101111110, //F
	0b0000100101111110  //G
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

void setchr(uint8_t pos, uint8_t num) {
	uint8_t saddr = pos*(D_HEIGHT);
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
		send_bytes(LED_CTRL0, ibyte);
	}
	sei();
	cli();
	for (uint8_t i = 0; i<5*3*3; i++) {
		uint8_t ibyte = current_pixels[i+15*3];
		send_bytes(LED_CTRL1, ibyte);
	}
	sei();
	cli();
	for (uint8_t i = 0; i<5*3*3; i++) {
		uint8_t ibyte = current_pixels[i+30*3];
		send_bytes(LED_CTRL2, ibyte);
	}
	sei();
	cli();
	for (uint8_t i = 0; i<5*3*3; i++) {
		uint8_t ibyte = current_pixels[i+45*3];
		send_bytes(LED_CTRL3, ibyte);
	}
	sei();
	cli();
	for (uint8_t i = 0; i<5*3*3; i++) {
		uint8_t ibyte = current_pixels[i+60*3];
		send_bytes(LED_CTRL4, ibyte);
	}
	sei();
	cli();
	for (uint8_t i = 0; i<5*2*3; i++) {
		uint8_t ibyte = current_pixels[i+75*3];
		send_bytes(LED_CTRL5, ibyte);
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


