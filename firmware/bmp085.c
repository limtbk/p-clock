/*
 * bmp085.c
 *
 *  Created on: Dec 26, 2015
 *      Author: lim
 */

#include "bmp085.h"
#include "timer1.h"

#define I2C_READ 1
#define I2C_WRITE 0

void bmp085_writemem(uint8_t reg, uint8_t value) {
	i2c_start(BMP085_ADDR | I2C_WRITE);
	i2c_write(reg);
	i2c_write(value);
	i2c_stop();
}

void bmp085_readmem(uint8_t reg, uint8_t buff[], uint8_t bytes) {
	uint8_t i = 0;
	i2c_start(BMP085_ADDR | I2C_WRITE);
	i2c_write(reg);
	i2c_start(BMP085_ADDR | I2C_READ);
	for (i=0; i<bytes; i++) {
		if (i==bytes-1)
			buff[i] = i2c_readNak();
		else
			buff[i] = i2c_readAck();
	}
	i2c_stop();
}

#if BMP085_FILTERPRESSURE == 1
#define BMP085_AVARAGECOEF 21
static int32_t k[BMP085_AVARAGECOEF];
int32_t bmp085_avaragefilter(int32_t input) {
	uint8_t i=0;
	int32_t sum=0;
	for (i=0; i<BMP085_AVARAGECOEF; i++) {
		k[i] = k[i+1];
	}
	k[BMP085_AVARAGECOEF-1] = input;
	for (i=0; i<BMP085_AVARAGECOEF; i++) {
		sum += k[i];
	}
	return (sum / BMP085_AVARAGECOEF) ;
}
#endif

/*
 * read calibration registers
 */
void bmp085_getcalibration() {
	uint8_t buff[2] = {0, 0};

	bmp085_readmem(BMP085_REGAC1, buff, 2);
	bmp085_regac1 = ((int16_t)buff[0] << 8 | ((int16_t)buff[1]));
	bmp085_readmem(BMP085_REGAC2, buff, 2);
	bmp085_regac2 = ((int16_t)buff[0] << 8 | ((int16_t)buff[1]));
	bmp085_readmem(BMP085_REGAC3, buff, 2);
	bmp085_regac3 = ((int16_t)buff[0] << 8 | ((int16_t)buff[1]));
	bmp085_readmem(BMP085_REGAC4, buff, 2);
	bmp085_regac4 = ((uint16_t)buff[0] << 8 | ((uint16_t)buff[1]));
	bmp085_readmem(BMP085_REGAC5, buff, 2);
	bmp085_regac5 = ((uint16_t)buff[0] << 8 | ((uint16_t)buff[1]));
	bmp085_readmem(BMP085_REGAC6, buff, 2);
	bmp085_regac6 = ((uint16_t)buff[0] << 8 | ((uint16_t)buff[1]));
	bmp085_readmem(BMP085_REGB1, buff, 2);
	bmp085_regb1 = ((int16_t)buff[0] << 8 | ((int16_t)buff[1]));
	bmp085_readmem(BMP085_REGB2, buff, 2);
	bmp085_regb2 = ((int16_t)buff[0] << 8 | ((int16_t)buff[1]));
	bmp085_readmem(BMP085_REGMB, buff, 2);
	bmp085_regmb = ((int16_t)buff[0] << 8 | ((int16_t)buff[1]));
	bmp085_readmem(BMP085_REGMC, buff, 2);
	bmp085_regmc = ((int16_t)buff[0] << 8 | ((int16_t)buff[1]));
	bmp085_readmem(BMP085_REGMD, buff, 2);
	bmp085_regmd = ((int16_t)buff[0] << 8 | ((int16_t)buff[1]));
}

/*
 * get raw temperature as read by registers, and do some calculation to convert it
 */
void bmp085_getrawtemperature() {
	uint8_t buff[2] = {0, 0};
	int32_t ut, x1, x2;

	//read raw temperature
	bmp085_writemem(BMP085_REGCONTROL, BMP085_REGREADTEMPERATURE);
	timer1_delay_ms(5); // min. 4.5ms read Temp delay
	bmp085_readmem(BMP085_REGCONTROLOUTPUT, buff, 2);
	ut = ((int32_t)buff[0] << 8 | ((int32_t)buff[1])); //uncompensated temperature value

	//calculate raw temperature
	x1 = ((int32_t)ut - bmp085_regac6) * bmp085_regac5 >> 15;
	x2 = ((int32_t)bmp085_regmc << 11) / (x1 + bmp085_regmd);
	bmp085_rawtemperature = x1 + x2;
}

/*
 * get raw pressure as read by registers, and do some calculation to convert it
 */
void bmp085_getrawpressure() {
	uint8_t buff[3] = {0, 0, 0};
	int32_t up, x1, x2, x3, b3, b6, p;
	uint32_t b4, b7;

	#if BMP085_AUTOUPDATETEMP == 1
	bmp085_getrawtemperature();
	#endif

	//read raw pressure
	bmp085_writemem(BMP085_REGCONTROL, BMP085_REGREADPRESSURE + (BMP085_MODE << 6));
	timer1_delay_ms(2 + (3 << BMP085_MODE));
	bmp085_readmem(BMP085_REGCONTROLOUTPUT, buff, 3);
	up = ((((int32_t)buff[0] << 16) | ((int32_t)buff[1] << 8) | ((int32_t)buff[2])) >> (8 - BMP085_MODE)); // uncompensated pressure value

	//calculate raw pressure
	b6 = bmp085_rawtemperature - 4000;
	x1 = (bmp085_regb2* (b6 * b6) >> 12) >> 11;
	x2 = (bmp085_regac2 * b6) >> 11;
	x3 = x1 + x2;
	b3 = (((((int32_t)bmp085_regac1) * 4 + x3) << BMP085_MODE) + 2) >> 2;
	x1 = (bmp085_regac3 * b6) >> 13;
	x2 = (bmp085_regb1 * ((b6 * b6) >> 12)) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	b4 = (bmp085_regac4 * (uint32_t)(x3 + 32768)) >> 15;
	b7 = ((uint32_t)up - b3) * (50000 >> BMP085_MODE);
	p = b7 < 0x80000000 ? (b7 << 1) / b4 : (b7 / b4) << 1;
	x1 = (p >> 8) * (p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;
	bmp085_rawpressure = p + ((x1 + x2 + 3791) >> 4);

	#if BMP085_FILTERPRESSURE == 1
	bmp085_rawpressure = bmp085_avaragefilter(bmp085_rawpressure);
	#endif
}

/*
 * get celsius temperature
 */
int8_t bmp085_gettemperature() {
	bmp085_getrawtemperature();
	int32_t temperature = ((bmp085_rawtemperature + 8)>>4);
	temperature = temperature / 10;
	return (int8_t)temperature;
}

/*
 * get pressure
 */
int32_t bmp085_getpressure() {
	bmp085_getrawpressure();
	return bmp085_rawpressure + BMP085_UNITPAOFFSET;
}

/*
 * init bmp085
 */
void bmp085_init() {
	#if BMP085_I2CINIT == 1
	i2c_init();
	timer1_delay_ms(10);
	#endif

	bmp085_getcalibration(); //get calibration data
	bmp085_getrawtemperature(); //update raw temperature, at least the first time

	#if BMP085_FILTERPRESSURE == 1
	//initialize the avarage filter
	uint8_t i=0;
	for (i=0; i<BMP085_AVARAGECOEF; i++) {
		bmp085_getrawpressure();
	}
	#endif
}


