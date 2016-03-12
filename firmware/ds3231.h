/*
 * ds3231.h
 *
 *  Created on: Oct 27, 2014
 *      Author: lim
 */

#ifndef DS13XX_H_
#define DS13XX_H_

#include <avr/io.h>

typedef struct TimeStruct {
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
} TTime;

typedef struct DateStruct {
	uint8_t weekday;
	uint8_t day;
	uint8_t month;
	uint8_t year;
} TDate;

typedef struct TemperatureStruct {
	uint8_t intPart;
	uint8_t fracPart;
} TTemperature;

TTime ttime(uint8_t hour, uint8_t min, uint8_t sec);
TDate tdate(uint8_t weekday, uint8_t day, uint8_t month, uint8_t year);
uint8_t ds3231_settime(TTime time, uint8_t bcd);
uint8_t ds3231_gettime(TTime *time, uint8_t bcd);
uint8_t ds3231_setdate(TDate date, uint8_t bcd);
uint8_t ds3231_getdate(TDate *date, uint8_t bcd);
uint32_t ds3231_gettimestamp();
uint8_t ds3231_gettemperature(TTemperature *temperature, uint8_t bcd);
uint8_t ds3231_readbyte(uint8_t addr);
uint8_t ds3231_writebyte(uint8_t addr, uint8_t byte);
uint8_t ds3231_readbytes(uint8_t addr, void *dst, uint8_t count);
uint8_t ds3231_writebytes(uint8_t addr, void *src, uint8_t count);

uint16_t timetoseconds(TTime time);
void secondstotime(uint16_t seconds, TTime *time);
uint16_t datetodays(TDate date);
void daystodate(uint16_t days, TDate *date);
uint32_t datetimetotimestamp(TDate date, TTime time);
void timestamptodatetime(uint32_t timestamp, TDate *date, TTime *time);

uint8_t dectobin(uint8_t dec);
uint8_t bintodec(uint8_t bin);

#endif /* DS13XX_H_ */
