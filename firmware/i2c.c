/*
 * i2c.c
 *
 *  Created on: Oct 22, 2014
 *      Author: lim
 */
#include <compat/twi.h>
#include "i2c.h"

#ifndef SCL_CLOCK
#define SCL_CLOCK 100000L
#endif

void i2c_init() {
	  TWSR = 0;
	  TWBR = ((F_CPU/SCL_CLOCK)-16)/2;
}

uint8_t i2c_start(uint8_t address) //and re-start
{
    uint8_t   twst;
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN); // send START condition
	while(!(TWCR & (1<<TWINT))); // wait until transmission completed
	twst = TW_STATUS & 0xF8; // check value of TWI Status Register. Mask prescaler bits.
	if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;
	TWDR = address; // send device address
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT))); // wail until transmission completed and ACK/NACK has been received
	twst = TW_STATUS & 0xF8; // check value of TWI Status Register. Mask prescaler bits.
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 2;
	return 0;
}

void i2c_stop(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO); // send stop condition
	while(TWCR & (1<<TWSTO)); // wait until stop condition is executed and bus released
}

uint8_t i2c_write(uint8_t data)
{
    uint8_t   twst;
	TWDR = data; // send data to the previously addressed device
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT))); // wait until transmission completed
	twst = TW_STATUS & 0xF8; // check value of TWI Status Register. Mask prescaler bits
	if( twst != TW_MT_DATA_ACK) return 1;
	return 0;

}

uint8_t i2c_readAck(void) //read byte
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR & (1<<TWINT)));
    return TWDR;
}

uint8_t i2c_readNak(void) //read last byte
{
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
    return TWDR;
}
