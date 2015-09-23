/*
 * i2c.h
 *
 *  Created on: Oct 21, 2014
 *      Author: lim
 */

#ifndef I2C_H_
#define I2C_H_

void i2c_init(void);
uint8_t i2c_start(uint8_t address); //and re-start
void i2c_stop(void);
uint8_t i2c_write(uint8_t data);
uint8_t i2c_readAck(void); //read byte
uint8_t i2c_readNak(void); //read last byte

#endif /* I2C_H_ */
