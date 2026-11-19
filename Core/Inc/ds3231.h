/*
 * ds3231.h
 *
 *  Created on: Nov 7, 2026
 *      Author: adi
 */

#ifndef INC_DS3231_H_
#define INC_DS3231_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"
//#include "time.h"

#define TIMER_ADDR 0x68 // Use 8-bit address
#define REG_TIME 0x00
#define REG_ENABLE_OSC 0x0E

struct t_1{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
};

void enable_oscillator_1(I2C_HandleTypeDef *hi2c);

// use this function only once.
void set_rtc_to_current_1(I2C_HandleTypeDef *hi2c, uint8_t sec, uint8_t minute, uint8_t hour, uint8_t dWeek, uint8_t date, uint8_t month, uint8_t year);


struct t_1 get_current_time_1(I2C_HandleTypeDef *hi2c);

uint32_t get_current_time_in_sec_1(I2C_HandleTypeDef *hi2c);

uint8_t get_temperature(I2C_HandleTypeDef *hi2c);


#ifdef __cplusplus
}
#endif


#endif /* INC_DS3231_H_ */
