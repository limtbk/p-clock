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

#ifdef I2C_DEBUG
uint16_t dbg[6] = {0, 0, 0, 0, 0, 0};
#endif

void i2c_init() {
	  TWSR = 0;
	  TWBR = ((F_CPU/SCL_CLOCK)-16)/2;
}

uint8_t i2c_start(uint8_t address) //and re-start
{
#ifdef I2C_DEBUG
	dbg[0] = 0;
	dbg[1] = 0;
#endif
    uint8_t twst;
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN); // send START condition
	while(!(TWCR & (1<<TWINT))) {
#ifdef I2C_DEBUG
		dbg[0]++;
		if (dbg[0]>65534) return 3;
#endif
	}; // wait until transmission completed
	twst = TW_STATUS & 0xF8; // check value of TWI Status Register. Mask prescaler bits.
	if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;
	TWDR = address; // send device address
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT))) {
#ifdef I2C_DEBUG
		dbg[1]++;
		if (dbg[1]>65534) return 4;
#endif
	}; // wail until transmission completed and ACK/NACK has been received
	twst = TW_STATUS & 0xF8; // check value of TWI Status Register. Mask prescaler bits.
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 2;
	return 0;
}

void i2c_stop(void)
{
#ifdef I2C_DEBUG
	dbg[2] = 0;
#endif
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO); // send stop condition
	while(TWCR & (1<<TWSTO)) {
#ifdef I2C_DEBUG
		dbg[2]++;
		if (dbg[2]>65534) return 5;
#endif
	}; // wait until stop condition is executed and bus released
}

uint8_t i2c_write(uint8_t data)
{
#ifdef I2C_DEBUG
	dbg[3] = 0;
#endif
    uint8_t twst;
	TWDR = data; // send data to the previously addressed device
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT))) {
#ifdef I2C_DEBUG
		dbg[3]++;
		if (dbg[3]>65534) return 6;
#endif
	}; // wait until transmission completed
	twst = TW_STATUS & 0xF8; // check value of TWI Status Register. Mask prescaler bits
	if( twst != TW_MT_DATA_ACK) return 1;
	return 0;
}

uint8_t i2c_readAck(void) //read byte
{
#ifdef I2C_DEBUG
	dbg[4] = 0;
#endif
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR & (1<<TWINT))) {
#ifdef I2C_DEBUG
		dbg[4]++;
		if (dbg[4]>65534) return 0;
#endif
	};
    return TWDR;
}

uint8_t i2c_readNak(void) //read last byte
{
#ifdef I2C_DEBUG
	dbg[5] = 0;
#endif
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT))) {
#ifdef I2C_DEBUG
		dbg[5]++;
		if (dbg[5]>65534) return 0;
#endif
	};
    return TWDR;
}

#ifdef I2C_DEBUG
uint16_t* dbginfo() {
	return dbg;
}
#endif
