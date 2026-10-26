/*
 * ds1307.h
 *
 *  Created on: Sep 30, 2026
 *      Author: adi
 */
#include "ds1307.h"

// use this function only once
void set_rtc_to_current(I2C_HandleTypeDef *hi2c, uint8_t sec, uint8_t minute, uint8_t hour, uint8_t dWeek, uint8_t date, uint8_t month, uint8_t year){


	uint8_t buff[8];
	// set buffer of timer address of ds1307
	// starts from 0x00 : seconds
	buff[0] = ((sec/10) << 4 | (sec % 10)) & 0x7f;

	// minutes: 0x01
	buff[1] = (minute/10) << 4 | (minute % 10);

	// hours: 0x02 (in 24 hour mode)
	buff[2] = ((hour/10) << 4 | (hour % 10)) & 0x3f;

	// day of week : 0x03
	buff[3] = dWeek & 0x07;

	// date
	buff[4] = (date/10) << 4 | (date % 10);

	// month
	buff[5] = (month/10) << 4 | (month % 10);

	// year
	buff[6] = (year/10) << 4 | (year % 10);

	HAL_I2C_Mem_Write(hi2c, (TIMER_ADDR << 1), REG_TIME, I2C_MEMADD_SIZE_8BIT, buff, 7, 100);

}

struct t_ get_current_time(I2C_HandleTypeDef *hi2c){

	uint8_t buff[3];

	HAL_I2C_Mem_Read(hi2c, (TIMER_ADDR << 1), REG_TIME, I2C_MEMADD_SIZE_8BIT, buff, 3, 100);


	struct t_ time_;
	time_.second = (uint8_t)(((buff[0]&0x7f) >> 4)*10 + (buff[0] & 0x0f));
	time_.minute = (uint8_t)(((buff[1]&0x7f) >> 4)*10 + (buff[1] & 0x0f));
	time_.hour = (uint8_t)(((buff[2]&0x3f) >> 4)*10 + (buff[2] & 0x0f));

	return time_;
}
