/*
 * timer1.h
 *
 *  Created on: Sep 27, 2014
 *      Author: lim
 */

#ifndef TIMER1_H_
#define TIMER1_H_

void timer1_init(void);
void timer1_delay_us2(uint16_t timeInterval);
void timer1_delay_ms(uint16_t timeInterval);
uint16_t timer1_measureTime(void);

#endif /* TIMER1_H_ */
