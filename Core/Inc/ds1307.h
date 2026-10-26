/*
 * ds1307.h
 *
 *  Created on: Sep 30, 2026
 *      Author: adi
 */

#ifndef INC_DS1307_H_
#define INC_DS1307_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"
//#include "time.h"

#define TIMER_ADDR 0x68 // Use 8-bit address
#define REG_TIME 0x00

struct t_{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
};

// use this function only once.
void set_rtc_to_current(I2C_HandleTypeDef *hi2c, uint8_t sec, uint8_t minute, uint8_t hour, uint8_t dWeek, uint8_t date, uint8_t month, uint8_t year);


struct t_ get_current_time();


#ifdef __cplusplus
}
#endif

#endif /* INC_DS1307_H_ */
