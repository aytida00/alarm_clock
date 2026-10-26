/*
 * tm1637.c
 *
 *  Created on: Sep 29, 2026
 *      Author: adi
 */

#include "tm1637.h"

void Delay_us(int time){
	for(int i = 0; i < time; i++){
		for(int j = 0; j < time; j++){
			__asm__("nop");
		}
	}
}

void CLK_HIGH(void){
	TM_CLK_GPIO_Port->BSRR = TM_CLK_Pin;
}

void CLK_LOW(void){
	TM_CLK_GPIO_Port->BSRR = TM_CLK_Pin << 16u;
}

void DATA_HIGH(void){
	TM_DIO_GPIO_Port->BSRR = TM_DIO_Pin;
}

void DATA_LOW(void){
	TM_DIO_GPIO_Port->BSRR = TM_DIO_Pin << 16u;
}



void start(void){
	CLK_HIGH();
	DATA_HIGH();
	Delay_us (2);
	DATA_LOW();
}

void stop (void)
{
	CLK_LOW();
	Delay_us (2);
	DATA_LOW();
	Delay_us (2);
	CLK_HIGH();
	Delay_us (2);
	DATA_HIGH();
}


void writeByte (uint8_t byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		CLK_LOW();
		if (byte & 0x01) // low front
		{
			DATA_HIGH();
		}
		else
		{
			DATA_LOW();
		}
		Delay_us (3);
		byte = byte >> 1;
		CLK_HIGH();
		Delay_us (3);
	}
}


void waitforAck (void)
{
	CLK_LOW();
	Delay_us (5); // After the falling edge of the eighth clock delay 5us
	              // ACK signals the beginning of judgment
//	while (dio);  // Check the state of the Data pin
	CLK_HIGH();
	Delay_us (2);
	CLK_LOW();
}

void TM1637_WriteData (uint8_t Addr, uint8_t *data, int size)
{
	start();
	writeByte(0x40);
	waitforAck();
	stop();
	start();
	writeByte(Addr);
	waitforAck();
	for (int i=0; i<size; i++)
	{
		writeByte(data[i]);
		waitforAck();
	}
	stop();
	start();
	writeByte(0x88);
	waitforAck();
	stop();
}

// a method to diplay all 4 digits given as input
// @param all_digit is a pointer to an array of size 4 of type uint8_t.
void display_all_digits(uint8_t * all_digit){
	uint8_t data[4];
	// run a loop for all 4 digits
	for (uint8_t i = 0; i < 4; i++){
		switch (all_digit[i]){
		case 0:
			data[i] = DIGIT_0;
			break;
		case 1:
			data[i] = DIGIT_1;
			break;
		case 2:
			data[i] = DIGIT_2;
			break;
		case 3:
			data[i] = DIGIT_3;
			break;
		case 4:
			data[i] = DIGIT_4;
			break;
		case 5:
			data[i] = DIGIT_5;
			break;
		case 6:
			data[i] = DIGIT_6;
			break;
		case 7:
			data[i] = DIGIT_7;
			break;
		case 8:
			data[i] = DIGIT_8;
			break;
		case 9:
			data[i] = DIGIT_9;
			break;
		default:
			data[i] = 0x00;
		}
	}
	// display colon sign also by changing bit of data[2]'s 7th bit to 1.
	data[1] |= 0x80;

	TM1637_WriteData(0xc0, data, 4);
}

void display_all_chars(char *all_chars){
	uint8_t data[4];
	// run a loop for all 4 digits
		for (uint8_t i = 0; i < 4; i++){
			switch (all_chars[i]){
			case 'A':
				data[i] = ALPHABET_A;
				break;
			case 'L':
				data[i] = ALPHABET_L;
				break;
			case 'C':
				data[i] = ALPHABET_C;
				break;
			case 't':
				data[i] = ALPHABET_t;
				break;
			case 'i':
				data[i] = ALPHABET_i;
				break;
			case 'S':
				data[i] = ALPHABET_S;
				break;
			default:
				data[i] = 0x00;
			}
		}

		TM1637_WriteData(0xc0, data, 4);
}

