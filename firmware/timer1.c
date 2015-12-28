/*
 * timer1.c
 *
 *  Created on: Sep 27, 2014
 *      Author: lim
 */
#include <avr/interrupt.h>
#include "timer1.h"
#include "ports.h"

uint16_t timer1_lastTime;

void timer1_init(void) {
#if defined (__AVR_ATmega32U4__)
	TCCR1A = (0<<COM1A1)|(0<<COM1A0)|(0<<COM1B1)|(0<<COM1B0)|(0<<COM1C1)|(0<<COM1C0)|(0<<WGM11)|(0<<WGM10);
	TCCR1B = (0<<ICNC1)|(0<<ICES1)|(0<<WGM13)|(0<<WGM12)|(0<<CS12)|(1<<CS11)|(0<<CS10); //Normal, clk/8
	TCCR1C = (0<<FOC1A)|(0<<FOC1B)|(0<<FOC1C);
	TIMSK1 = (0<<ICIE1)|(0<<OCIE1C)|(0<<OCIE1B)|(0<<OCIE1A)|(0<<TOIE1);
#elif defined (__AVR_ATmega328P__)
	TCCR1A = (0<<COM1A1)|(0<<COM1A0)|(0<<COM1B1)|(0<<COM1B0)|(0<<WGM11)|(0<<WGM10);
	TCCR1B = (0<<ICNC1)|(0<<ICES1)|(0<<WGM13)|(0<<WGM12)|(0<<CS12)|(1<<CS11)|(0<<CS10); //Normal, clk/8
	TIMSK0 = (0<<ICIE1)|(0<<OCIE1B)|(0<<OCIE1A)|(0<<TOIE1);
#else
	TCCR1A = (0<<COM1A1)|(0<<COM1A0)|(0<<COM1B1)|(0<<COM1B0)|(0<<WGM11)|(0<<WGM10);
	TCCR1B = (0<<ICNC1)|(0<<ICES1)|(0<<WGM13)|(0<<WGM12)|(0<<CS12)|(1<<CS11)|(0<<CS10); //Normal, clk/8
	TIMSK = (0<<TICIE1)|(0<<OCIE1B)|(0<<OCIE1A)|(0<<TOIE1);
#endif
}

void resetTimer1(void) {
	TCNT1 = 0;
}

uint16_t timer1_measureTime(void) {
	uint16_t time = TCNT1;
	uint16_t result = time - timer1_lastTime;
	timer1_lastTime = time;
	return result;
}

void timer1_delay_us2(uint16_t timeInterval) {
	timeInterval = timeInterval;
	uint16_t startTime = TCNT1;
	uint16_t currentTime = TCNT1;
	while ((currentTime - startTime) < timeInterval) {
		currentTime = TCNT1;
	}
}

void timer1_delay_ms(uint16_t timeInterval) {
	for (uint16_t t = 0; t < timeInterval; t++) {
		timer1_delay_us2(2000);
	}
}

ISR(TIMER1_CAPT_vect)
{
}

ISR(TIMER1_COMPA_vect)
{
}

ISR(TIMER1_COMPB_vect)
{
}

ISR(TIMER1_OVF_vect)
{
}
