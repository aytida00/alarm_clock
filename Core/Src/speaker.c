#include "speaker.h"


#include "speaker.h"

void speaker_play(TIM_HandleTypeDef *htim, uint16_t freq) {
    // Avoid division by zero just in case
    if (freq == 0) {
        speaker_stop(htim);
        return;
    }

    // 1. Calculate and set the new ARR using the generic handle pointer
    uint32_t new_arr = (1000000 / freq) - 1;
    htim->Instance->ARR = new_arr;

    // 2. Set the duty cycle exactly to 50% of the new ARR
    htim->Instance->CCR1 = new_arr / 20;

    // 3. Force an update event to apply ARR and CCR1 changes instantly
    // This prevents any glitching or overshooting when switching frequencies
    htim->Instance->EGR = TIM_EGR_UG;

    // 4. Start the PWM generation
    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1);
}

void speaker_stop(TIM_HandleTypeDef *htim) {
    HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_1);
}

