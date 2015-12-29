/*
 * dht11.c
 *
 *  Created on: Dec 28, 2015
 *      Author: lim
 */     

#include "dht11.h"
#include "timer1.h"
        
uint8_t dht11_getdata(uint8_t select) {
	uint8_t bits[5] = {0, 0, 0, 0, 0};
	uint8_t i = 0;
	uint8_t j = 0;

	//reset port
	SETD(DHT11);
	SETP(DHT11);
	timer1_delay_ms(100);

	//send request
	CLRP(DHT11);
	timer1_delay_ms(18);
	SETP(DHT11);
	timer1_delay_us2(2);
	CLRD(DHT11);
	timer1_delay_us2(78);

	//check start condition 1
	if (PINV(DHT11)) {
		return DHT_ERROR;
	}
	timer1_delay_us2(160);
	//check start condition 2
	if (!PINV(DHT11)) {
		return DHT_ERROR;
	}
	timer1_delay_us2(160);

	//read the data
	for (j=0; j<5; j++) { //read 5 byte
		uint8_t result=0;
		for(i=0; i<8; i++) {//read every bit
			while (!PINV(DHT11)); //wait for an high input
			timer1_delay_us2(60);
			if (PINV(DHT11)) //if input is high after 30 us, get result
				result |= (1<<(7-i));
			while (PINV(DHT11)); //wait until input get low
		}
		bits[j] = result;
	}

	//reset port
	SETD(DHT11);
	SETP(DHT11);
	timer1_delay_ms(100);

	//check checksum
	if (bits[0] + bits[1] + bits[2] + bits[3] == bits[4]) {
		if (select == 0) { //return temperature
			return (bits[2]);
		} else if (select == 1){ //return humidity
			return (bits[0]);
		}
	}

	return DHT_ERROR;
}
        
int8_t dht11_gettemperature() {
	return dht11_getdata(0);
}
        
int8_t dht11_gethumidity() {
	return dht11_getdata(1);
}
