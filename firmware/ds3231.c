/*
 * ds3231.c
 *
 *  Created on: Oct 27, 2014
 *      Author: lim
 */

#include "ds3231.h"
#include "i2c.h"

#define DS13xx_I2C_ADDRESS 0x68

TTime ttime(uint8_t hour, uint8_t min, uint8_t sec) {
	TTime result;
	result.sec = sec;
	result.min = min;
	result.hour = hour;
	return result;
}

TDate tdate(uint8_t weekday, uint8_t day, uint8_t month, uint8_t year) {
	TDate result;
	result.weekday = weekday;
	result.day = day;
	result.month = month;
	result.year = year;
	return result;
}

uint8_t ds3231_settime(TTime time, uint8_t bcd) {
	uint8_t res = 0;
	res = i2c_start(DS13xx_I2C_ADDRESS << 1);
	if (res == 0) {
		res |= i2c_write(0);
		TTime wtime = time;
		if (bcd == 0) {
			wtime.sec = bintodec(wtime.sec);
			wtime.min = bintodec(wtime.min);
			wtime.hour = bintodec(wtime.hour);
		}
		res |= i2c_write(wtime.sec);
		res |= i2c_write(wtime.min);
		res |= i2c_write(wtime.hour);
		i2c_stop();
	}
	return res;
}

uint8_t ds3231_gettime(TTime *time, uint8_t bcd) {
	uint8_t res = 0;
	res = i2c_start(DS13xx_I2C_ADDRESS << 1);
	if (res == 0) {
		res |= i2c_write(0);
		res |= i2c_start((DS13xx_I2C_ADDRESS << 1) | 0x01);
		time->sec = i2c_readAck();
		time->min = i2c_readAck();
		time->hour = i2c_readNak();
		if (bcd == 0) {
			time->sec = dectobin(time->sec);
			time->min = dectobin(time->min);
			time->hour = dectobin(time->hour);
		}
		i2c_stop();
	}
	return res;
}

uint32_t ds3231_gettimestamp() {
	TTime time;
	TDate date;
	uint8_t res = ds3231_gettime(&time, 1);
	if (!res) {
		res = ds3231_getdate(&date, 1);
	}
	uint32_t result = datetimetotimestamp(date, time);
	return result;
}

uint8_t ds3231_setdate(TDate date, uint8_t bcd) {
	uint8_t res = 0;
	res = i2c_start(DS13xx_I2C_ADDRESS << 1);
	if (res == 0) {
		res |= i2c_write(3);
		TDate wdate = date;
		if (bcd == 0) {
			wdate.day = bintodec(wdate.day);
			wdate.month = bintodec(wdate.month);
			wdate.year = bintodec(wdate.year);
		}
		res |= i2c_write(wdate.weekday);
		res |= i2c_write(wdate.day);
		res |= i2c_write(wdate.month);
		res |= i2c_write(wdate.year);
		i2c_stop();
	}
	return res;
}

uint8_t ds3231_getdate(TDate *date, uint8_t bcd) {
	uint8_t res = 0;
	res = i2c_start(DS13xx_I2C_ADDRESS << 1);
	if (res == 0) {
		res |= i2c_write(3);
		res |= i2c_start((DS13xx_I2C_ADDRESS << 1) | 0x01);
		date->weekday = i2c_readAck();
		date->day = i2c_readAck();
		date->month = i2c_readAck();
		date->year = i2c_readNak();
		if (bcd == 0) {
			date->day = dectobin(date->day);
			date->month = dectobin(date->month);
			date->year = dectobin(date->year);
		}
		i2c_stop();
	}
	return res;
}

uint8_t ds3231_gettemperature(TTemperature *temperature, uint8_t bcd) {
	uint8_t res = 0;
	res = i2c_start(DS13xx_I2C_ADDRESS << 1);
	if (res == 0) {
		res |= i2c_write(0x11);
		res |= i2c_start((DS13xx_I2C_ADDRESS << 1) | 0x01);
		temperature->intPart = i2c_readAck();
		temperature->fracPart = i2c_readNak();
		if (bcd != 0) {
			temperature->intPart = bintodec(temperature->intPart);
			temperature->fracPart = bintodec(temperature->fracPart);
		}
		i2c_stop();
	}
	return res;
}

uint8_t ds3231_readbyte(uint8_t addr) {
	uint8_t res = 0;
	uint8_t result = 0;
	res = i2c_start(DS13xx_I2C_ADDRESS << 1);
	if (res == 0) {
		res |= i2c_write(addr);
		res |= i2c_start((DS13xx_I2C_ADDRESS << 1) | 0x01);
		result = i2c_readNak();
		i2c_stop();
	}
	return result;
}

uint8_t ds3231_writebyte(uint8_t addr, uint8_t byte) {
	uint8_t res = 0;
	res = i2c_start(DS13xx_I2C_ADDRESS << 1);
	if (res == 0) {
		res |= i2c_write(addr);
		res |= i2c_write(byte);
		i2c_stop();
	}
	return res;
}

uint8_t ds3231_readbytes(uint8_t addr, void *dst, uint8_t count) {
	uint8_t res = 0;
	if (count>0) {
		res = i2c_start(DS13xx_I2C_ADDRESS << 1);
		if (res == 0) {
			res |= i2c_write(addr);
			res |= i2c_start((DS13xx_I2C_ADDRESS << 1) | 0x01);
			uint8_t *dest = dst;
			uint8_t i;
			for (i = 0; i < count-1; i++) {
				dest[i] = i2c_readAck();
			}
			dest[i] = i2c_readNak();
			i2c_stop();
		}
	}
	return res;
}

uint8_t ds3231_writebytes(uint8_t addr, void *src, uint8_t count) {
	uint8_t res = 0;
	res = i2c_start(DS13xx_I2C_ADDRESS << 1);
	if (res == 0) {
		res |= i2c_write(addr);
		uint8_t *source = src;
		for (uint8_t i = 0; i < count-1; i++) {
			res |= i2c_write(source[i]);
		}
		i2c_stop();
	}
	return res;
}

uint8_t dectobin(uint8_t dec) {
	uint8_t bin = (dec & 0x0F) + ((dec & 0xF0) >> 4) * 10;
	return bin;
}

uint8_t bintodec(uint8_t bin) {
	uint8_t dec = (bin % 10) + (((bin / 10) % 10) * 16);
	return dec;
}

uint16_t timetoseconds(TTime time) {
	int16_t result = dectobin(time.sec) + dectobin(time.min)*60 + dectobin(time.hour)*3600;
	return result;
}

void secondstotime(uint16_t seconds, TTime *time) {
	uint8_t sec = seconds % 60;
	uint8_t min = (seconds / 60) % 60;
	uint8_t hour = seconds / 3600;
	time->sec = bintodec(sec);
	time->min = bintodec(min);
	time->hour = bintodec(hour);
}

uint8_t dayofweek(TDate date) {
	int16_t y = dectobin(date.year) + 2000;
	int16_t m = dectobin(date.month);
	int16_t d = dectobin(date.day);
	int16_t weekday = (d+=m<3?y--:y-2,23*m/9+d+4+y/4-y/100+y/400)%7; //This One Liner Solution for "C" was given by Michael Keith and Tom Craver
	return (uint8_t)weekday;
}

static uint8_t month_leap [] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static uint8_t month_common [] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

uint8_t leapyear(uint8_t year) {
	uint8_t result = ((year % 4) == 0) && ((year % 100) != 0);
	return result;
}

uint16_t daysinyear(uint8_t year) {
	uint16_t result = (((year % 4) == 0) && ((year % 100) != 0)) ? 366 : 365;
	return result;
}

uint16_t datetodays(TDate date) { //days from 1 JAN 2000, so 1 is 2 JAN 2000
	uint16_t days = dectobin(date.day) - 1;
	uint8_t *monthdays = (leapyear(dectobin(date.year))) ? month_leap : month_common;

	for (int8_t i = 0; i < dectobin(date.month) - 1; i++) {
		days += monthdays[i];
	}
	for (uint8_t i = 0; i < dectobin(date.year); i++) {
		days += daysinyear(i);
	}
	return days;
}

void daystodate(uint16_t days, TDate *date) {
	uint8_t year = 0;
	uint16_t days_s = days;
	while (days_s >= daysinyear(year)) {
		days_s -= daysinyear(year);
		year++;
	}
	uint8_t *monthdays;
	if (leapyear(year)) {
		monthdays = month_leap;
	} else {
		monthdays = month_common;
	}
	uint8_t month = 0;
	while (days_s >= monthdays[month]) {
		days_s -= monthdays[month];
		month++;
	}
	date->day = bintodec(days_s + 1);
	date->month = bintodec(month + 1);
	date->year = bintodec(year);
	date->weekday = dayofweek(*date);
}

uint32_t datetimetotimestamp(TDate date, TTime time) {
	uint32_t result = timetoseconds(time);
	result += datetodays(date)*86400;
	return result;
}

void timestamptodatetime(uint32_t timestamp, TDate *date, TTime *time) {
	int32_t days = timestamp / 86400;
	int32_t seconds = timestamp % 86400;
	secondstotime((uint16_t)seconds, time);
	daystodate((uint16_t)days, date);
}

