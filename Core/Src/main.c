/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "tm1637.h"
#include "ds1307.h"
#include "speaker.h"
#include "stdbool.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
struct t_ start_t;

// for current time
uint32_t downtime_seconds;
uint32_t downtime_seconds_prev = 0;
uint32_t today_time_in_sec = 0;
uint8_t time_display_data[4];

// joystick
enum Joy_Commands {
	JOY_U,
	JOY_R,
	JOY_D,
	JOY_L,
	JOY_IDLE
};
bool joystick_at_idle = true;
uint32_t value_adc[2];


// For switch cases
volatile bool switch_pressed = false;
bool prev_switch_status = false;
bool long_switch_pressed = false;
uint32_t HIGH_TICK = 0, LOW_TICK = 0;
uint8_t double_quick_switch_counter = 0, single_quick_switch_counter = 0;
uint32_t single_switch_timer = 0;
bool enable_single_switch_timer = false;


int8_t current_function = 0;
int8_t current_layer = 0;
int8_t prev_layer = 0; // just for displaying alarm function(that is 0th function) when entered into layer1 from 0th layer

// layer: 1
int prev_joy_command = JOY_IDLE;

// layer: 2 --> TIMER
int8_t current_timer_option = 0;
bool stop_beep_beep_FLAG = false;

// For auto display off
bool auto_display_off_FLAG = false;
uint32_t auto_display_off_timer = 0;
bool auto_display_off_ENABLE = true;  // in layer 1 and layer 2 we don't want screen to go off. This is where this should be false to stop auto display off feature


bool start_timer_FLAG = false;

// timer sound parameters
bool start_beep_beep_FLAG = false;  // becomes high whenever alarm or timer finishes
uint8_t time_gap_counter = 0; // if self%3 = 0 then 100ms, = 1 then 100ms, =2 then 1 second
uint16_t timer_sound_interval = 0;
uint32_t timer_sound_prev_time = 0;
bool flip_on_off_sound_FLAG = true;

// for small beep
bool small_beep_FLAG = false;

int16_t timer_seconds = 0;
uint32_t timer_PREV_time = 0, timer_FUTURE_time = 0;
uint8_t display_timer_data[4];
// ##################################################################################
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

void display_current_time();
int get_joystick_commands(void);
void display_stages(void);
void start_timer(uint8_t otpion);
void timer_beep_beep(void);
void detect_switch_cases_v2(void);
void update_layer(void);
void show_function_selector(void);
void timer_selector(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
//	HAL_StatusTypeDef ret;




  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */



  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  uint8_t data[4] = {0b01110111, 0b01111100, 0b00111001, 0b01011110};  // 1, 2, 3, 4
  TM1637_WriteData(0xC0, data, 4);

//  set_rtc_to_current(&hi2c1, 0, 14, 0, 4, 1, 10, 26);
//  HAL_Delay(1000);

  start_t = get_current_time(&hi2c1);
  uint8_t start_minute = start_t.minute;
  uint8_t start_hour = start_t.hour;
  uint8_t start_seconds = start_t.second;
  uint32_t start_seconds_total = start_hour * 3600 + start_minute * 60 + start_seconds;



  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)value_adc, 2);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // ################################################################################################
	  // calculating today's current time in seconds
	  downtime_seconds = HAL_GetTick()/1000;
	  today_time_in_sec = start_seconds_total + downtime_seconds;
	  // ################################################################################################


	  detect_switch_cases_v2();

	  update_layer();

	  if (downtime_seconds - auto_display_off_timer >= 10 && (!auto_display_off_FLAG && auto_display_off_ENABLE)){
		  auto_display_off_FLAG = true;
		  uint8_t nothing_digits[4] = {0, 0, 0, 0};
		  TM1637_WriteData(0xc0, nothing_digits, 4);
	  }

	  if (current_layer == 0) {
		  if (!auto_display_off_ENABLE) auto_display_off_ENABLE = true;
		  display_current_time();
	  }

	  else if (current_layer == 1) {
		  if (auto_display_off_ENABLE) auto_display_off_ENABLE = false;
		  show_function_selector();
	  }

	  else if (current_layer == 2) {
		  // timer
		  if (current_function == 1){
			  timer_selector();
		  }
	  }

	  else if (current_layer == 10){
		  start_timer(current_timer_option);
		  if (start_beep_beep_FLAG) timer_beep_beep();
	  }



	  // clear all switch cases
	  single_quick_switch_counter = 0;
	  double_quick_switch_counter = 0;
	  long_switch_pressed = 0;
	  prev_layer = current_layer;


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 2499;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MY_LED_GPIO_Port, MY_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, TM_CLK_Pin|TM_DIO_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : MY_LED_Pin */
  GPIO_InitStruct.Pin = MY_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MY_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SWITCH_Pin */
  GPIO_InitStruct.Pin = SWITCH_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SWITCH_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : TM_CLK_Pin TM_DIO_Pin */
  GPIO_InitStruct.Pin = TM_CLK_Pin|TM_DIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == SWITCH_Pin){
		switch_pressed = !HAL_GPIO_ReadPin(SWITCH_GPIO_Port, SWITCH_Pin);
	}
}



void detect_switch_cases_v2(void) {
    uint32_t current_tick = HAL_GetTick();

    // Read the volatile variable into a local variable once per loop
    // to prevent it from changing halfway through this function.
    bool current_switch = switch_pressed;

    // set timer for auto display off
    if (current_switch) {
    	auto_display_off_timer = HAL_GetTick()/1000;
    	auto_display_off_FLAG = false;
    }

//    // handle long_switch beep sound
//    if((current_tick - HIGH_TICK >= 1500) && (HIGH_TICK - LOW_TICK > 50) && !small_beep_FLAG){
//    	small_beep_FLAG = true;
//    	speaker_play(&htim2, 350);
//    }

    // 1. FALLING/RISING EDGE DETECTION WITH 50ms DEBOUNCE
    if (current_switch && !prev_switch_status && (current_tick - LOW_TICK > 50)) {
        HIGH_TICK = current_tick; // Switch pressed
    }
    else if (!current_switch && prev_switch_status && (current_tick - HIGH_TICK > 50)) {
        LOW_TICK = current_tick;  // Switch released

        uint32_t press_duration = LOW_TICK - HIGH_TICK;

        // 2. CHECK IF SHORT PRESS OR LONG PRESS
        if (press_duration < 1500) {

            // If we were already waiting for a second click, this is a DOUBLE CLICK
            if (enable_single_switch_timer) {
                double_quick_switch_counter += 1;
                enable_single_switch_timer = false; // Cancel the single click timer
            }
            // Otherwise, this is the first click. Start the timer to wait for a potential second click.
            else {
                single_switch_timer = LOW_TICK;
                enable_single_switch_timer = true;
            }
        }
        else {
            // It was a long press, and it has just been released
//            LONG_SWITCH_BEEPED = false;  // Reset for next time
            long_switch_pressed = true;
            small_beep_FLAG = false;
            // Optional safety: if user released between 1500ms and 1800ms, stop the beep early
            if (small_beep_FLAG) {
            	small_beep_FLAG = false;
                speaker_stop(&htim2);
            }
        }
    }


    // 3. HANDLE SINGLE CLICK TIMEOUT
    // If 300ms have passed since a short press and no second press happened, it's a SINGLE CLICK
    if (enable_single_switch_timer && (current_tick - single_switch_timer > 300)) {
        single_quick_switch_counter += 1;
        enable_single_switch_timer = false;
    }

    // 4. HANDLE LONG PRESS 300ms BEEP
    // Step A: Start beep at 1500ms while button is actively held down
    if (current_switch && (current_tick - HIGH_TICK >= 1500) && !small_beep_FLAG) {
        small_beep_FLAG = true;    // Tracks that the buzzer is currently active
        speaker_play(&htim2, 600);
    }

    // Step B: Stop beep 300ms later (at 1800ms) even if user is still holding the button
    if (small_beep_FLAG && (current_tick - HIGH_TICK >= 1650)) {
        small_beep_FLAG = false;
        speaker_stop(&htim2);
    }

    	// Update previous state for the next loop iteration
    prev_switch_status = current_switch;
}


void update_layer(void){
//	current_layer - a global_var
	// from 0th layer to 1st layer
	if (current_layer == 0 && long_switch_pressed){
		current_function = 0; // that is alarm function
		// here I would need long switch press for forward pass
		current_layer = 1;  // entering to layer where functions can be selected
	}
	else if (current_layer == 1 && single_quick_switch_counter){
		current_layer = 2;  // Entering to particular function
	}
	else if (double_quick_switch_counter && (current_layer <= 2)){
		current_layer -= 1;
	}
	// timer option selected
	else if (current_layer == 2 && single_quick_switch_counter){
		if (current_function == 1){
			current_layer = 10; // specific for timer
			auto_display_off_ENABLE = true;
		}
	}
	// timer is finished, stop beeep-beep
	else if (current_layer == 10 && single_quick_switch_counter && start_beep_beep_FLAG){
		current_layer = 0;
		stop_beep_beep_FLAG = true;
		auto_display_off_ENABLE = true;
	}

	if (current_layer < 0) current_layer = 0;
}


void show_function_selector(void){
	// display functions according to current_function
	// display 0th function when entered into layer1
	if (current_layer != prev_layer){
		char display_data[4] = {'A', 'L', 0, 0};
		display_all_chars(display_data);
		current_function = 0;
	}
	// update screen only when joystick command change from previous global stored command
	int joy_command = get_joystick_commands();
	if (joy_command != prev_joy_command && (joy_command != JOY_IDLE)){
		char display_data[4] = {0, 0, 0, 0};
		if (joy_command == JOY_U && (current_function < 3)) current_function += 1;
		else if (joy_command == JOY_D && (current_function > 0)) current_function -= 1;

		switch (current_function){
				case 0:
					display_data[0] = 'A';
					display_data[1] = 'L';
					break;
				case 1:
					display_data[0] = 't';
					display_data[1] = 'i';
					break;
				case 2:
					display_data[0] = 'S';
					display_data[1] = 't';
					display_data[2] = 'C';
					display_data[3] = 'L';
					break;
				case 3:
					display_data[0] = 'C';
					display_data[1] = 'L';
					break;
				}

				display_all_chars(display_data);
	}
	prev_joy_command = joy_command;
}


void timer_selector(void){
	if (current_layer != prev_layer){
		uint8_t display_data[4] = {0, 0, 1, 0};
		display_all_digits(display_data);
		current_timer_option = 0;
	}
	// update screen only when joystick command change from previous global stored command
	int joy_command = get_joystick_commands();
	if (joy_command != prev_joy_command && (joy_command != JOY_IDLE)){
		uint8_t display_data[4] = {0, 0, 0, 0};
		if (joy_command == JOY_U && (current_timer_option < 3)) current_timer_option += 1;
		else if (joy_command == JOY_D && (current_timer_option > 0)) current_timer_option -= 1;

		switch (current_timer_option){
				case 0:                         // 10 minutes
					display_data[2] = 1;
					display_data[3] = 0;
					break;
				case 1:                        // 25 minutes
					display_data[2] = 2;
					display_data[3] = 5;
					break;
				case 2:                        // 45 minutes
					display_data[2] = 4;
					display_data[3] = 5;
					break;
				case 3:                        // 60 minutes
					display_data[0] = 0;
					display_data[1] = 1;
					break;
				}

				display_all_digits(display_data);
	}
	prev_joy_command = joy_command;

}



void display_current_time(){
	uint8_t current_hour = (uint8_t)(today_time_in_sec / 3600);
	uint8_t current_minute = (uint8_t)((today_time_in_sec - current_hour*3600)/60);
	uint8_t current_seconds = (uint8_t)(today_time_in_sec - current_hour*3600 - current_minute*60);


	time_display_data[0] = (current_minute / 10);
	time_display_data[1] = current_minute % 10;
	time_display_data[2] = (current_seconds / 10);
	time_display_data[3] = current_seconds % 10;


	if (downtime_seconds - downtime_seconds_prev >= 1){
		if (!auto_display_off_FLAG){
			display_all_digits(time_display_data);
		}
		downtime_seconds_prev = downtime_seconds;
	}
}



int get_joystick_commands(void){
	if (((value_adc[0] > 1000) && (value_adc[0] < 3000)) && ((value_adc[1] > 1000) && (value_adc[1] < 3000))){
		joystick_at_idle = true;
	}

	if (joystick_at_idle){
		// U
		if (value_adc[0] > 3500) {
			joystick_at_idle = false;
			return JOY_D;
		}
		// D
		if (value_adc[0] < 500){
			joystick_at_idle = false;
			return JOY_U;
		}




		// R
		if (value_adc[1] > 3500) {
			joystick_at_idle = false;
			return JOY_R;
		}
		// L
		if (value_adc[1] < 500){
			joystick_at_idle = false;
			return JOY_L;
		}


	}
	return 1000;  // just a random value other than [0:3]
}




void start_timer(uint8_t option){
	if (!start_timer_FLAG){
		switch (option){
		case 0:
			timer_seconds = 10*1;  // 10 minutes
			break;
		case 1:
			timer_seconds = 1*25; // 25 minutes
			break;
		case 3:
			timer_seconds = 1*45; // 45 minutes
			break;
		case 4:
			timer_seconds = 1*60; // 60 minutes
			break;
		}
		start_timer_FLAG = true;
		timer_FUTURE_time = today_time_in_sec + timer_seconds;
	}
	// start counting timer until 00:00
	else{
//		timer_CURRENT_time = HAL_GetTick()/1000;
		if (timer_FUTURE_time - today_time_in_sec > 0){
			// show the time
			if (today_time_in_sec - timer_PREV_time >= 1){
				// calculate the time and show it
				timer_seconds -= 1;
				uint8_t current_minute = (uint8_t)(timer_seconds / 60);
				uint8_t current_seconds = (uint8_t)(timer_seconds - current_minute*60);
				display_timer_data[0] = (current_minute / 10);
				display_timer_data[1] = current_minute % 10;
				display_timer_data[2] = (current_seconds / 10);
				display_timer_data[3] = current_seconds % 10;

				if (!auto_display_off_FLAG){
					display_all_digits(display_timer_data);
				}

				timer_PREV_time = today_time_in_sec;
			}
		}
		// timer over. Make a beep-beep(timer_sound).
		else{
			start_beep_beep_FLAG = true;  // play timer_beep_beep()
			timer_sound_prev_time = HAL_GetTick();
			start_timer_FLAG = false;
		}
	}
}


void timer_beep_beep(void){
	// check for user interrupt to stop sound
	if (stop_beep_beep_FLAG){
		stop_beep_beep_FLAG = false;
		start_beep_beep_FLAG = false;
		speaker_stop(&htim2);
		return;
	}

	// play beep-beep of high for 200ms and low for 200 ms then 1 second pause asynchronously.
	switch (time_gap_counter % 4){
	case 0: // on
		timer_sound_interval = 100;
		break;
	case 1: // off
		timer_sound_interval = 100;
		break;
	case 2: // on
		timer_sound_interval = 100;
		break;
	case 3: // off
		timer_sound_interval = 1000;
	}

	if (HAL_GetTick() - timer_sound_prev_time > timer_sound_interval){
		timer_sound_prev_time = HAL_GetTick();
		flip_on_off_sound_FLAG = true;
		time_gap_counter += 1;
	}

	if (flip_on_off_sound_FLAG){
		flip_on_off_sound_FLAG = false;
		if (time_gap_counter % 2 == 0){
			speaker_play(&htim2, 350);
			char mychar[4] = {'i', 'i', 'i', 'i'};
			display_all_chars(mychar);
		}
		else {
			speaker_stop(&htim2);
			char mychar[4] = {0, 0, 0, 0};
			display_all_chars(mychar);
		}

	}

}




//void display_stages(void){
//	if (current_stage != prev_stage){
//		char display_data[4] = {0, 0, 0, 0};
//		switch (current_stage){
//		case 0:
//			display_data[0] = 'C';
//			display_data[1] = 'L';
//			break;
//		case 1:
//			display_data[0] = 't';
//			display_data[1] = 'i';
//			break;
//		case 2:
//			display_data[0] = 'S';
//			display_data[1] = 't';
//			display_data[2] = 'C';
//			display_data[3] = 'L';
//			break;
//		case 3:
//			display_data[0] = 'A';
//			display_data[1] = 'L';
//			break;
//		}
//
//		display_all_chars(display_data);
//	}
//}



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
