/*
 * dht11.h
 *
 *  Created on: Dec 28, 2015
 *      Author: lim
 */

#ifndef DHT11_H_
#define DHT11_H_

#include <avr/io.h>
#include "ports.h"

#define DHT11 PORT_C2
#define DHT_ERROR 255

int8_t dht11_gettemperature();
int8_t dht11_gethumidity();

#endif /* DHT11_H_ */
