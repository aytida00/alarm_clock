/*
 * speaker.h
 *
 *  Created on: Oct 9, 2026
 *      Author: adi
 */

#ifndef INC_SPEAKER_H_
#define INC_SPEAKER_H_

#include "main.h"

void speaker_play(TIM_HandleTypeDef *htim, uint16_t freq);

void speaker_stop(TIM_HandleTypeDef *htim);

#endif /* INC_SPEAKER_H_ */
