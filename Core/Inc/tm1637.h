/*
 * tm1637.h
 *
 *  Created on: Sep 29, 2026
 *      Author: adi
 */

#ifndef INC_TM1637_H_
#define INC_TM1637_H_

#include "main.h"

// 7 segement display encoding
#define DIGIT_0 0x3f
#define DIGIT_1 0x06
#define DIGIT_2 0x5b
#define DIGIT_3 0x4f
#define DIGIT_4 0x66
#define DIGIT_5 0x6d
#define DIGIT_6 0x7d
#define DIGIT_7 0x07
#define DIGIT_8 0x7f
#define DIGIT_9 0x6f
#define ALPHABET_A 0x77
#define ALPHABET_L 0x38
#define ALPHABET_C 0x39
#define ALPHABET_t 0x78
#define ALPHABET_i 0x04
#define ALPHABET_S 0x6d

void start(void);

void stop(void);

void Delay_us(int time);

void CLK_HIGH(void);

void CLK_LOW(void);

void DATA_HIGH(void);

void DATA_LOW(void);

void writeByte (uint8_t byte);

void waitforAck (void);

void TM1637_WriteData (uint8_t Addr, uint8_t *data, int size);

// a method to diplay all 4 digits given as input
void display_all_digits(uint8_t * all_digit);

void display_all_chars(char *all_chars);

#endif /* INC_TM1637_H_ */
