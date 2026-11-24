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
#include "ds3231.h"
#include "speaker.h"
#include "at24cxx.h"
#include "stdbool.h"
#include "string.h"

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
struct t_1 start_t;

// for current time
uint32_t downtime_seconds;
uint32_t downtime_seconds_prev = 0;
uint32_t downtime_seconds_prev_HELPER = 0;
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

// lyaer: 2 --> TIMER
uint32_t my_alarm_playlist[4];
uint8_t total_enabled_alarms = 0;
bool start_playing_alarm = false;
bool compare_alarms = true;
bool currently_alarm_playing = false;


// layer: 2 --> TIMER
int8_t current_timer_option = 0;
bool stop_beep_beep_FLAG = false;

// layer: 2 --> STOP WATCH
uint8_t stop_w_data[4] = {0, 0, 0, 0};
uint32_t current_sw_tick = 0;
uint32_t prev_sw_tick = 0;
uint8_t stop_watch_layer = 0;
bool stop_watch_zero_display_FLAG = true;
uint32_t sw_counter = 0;

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

uint16_t timer_seconds = 0;
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
int get_joystick_commands_continuous(void);
void display_stages(void);
void start_timer(uint8_t otpion);
void timer_beep_beep(void);
void detect_switch_cases_v2(void);
void update_layer(void);
void show_function_selector(void);
void timer_selector(void);
void handle_stop_watch(void);
void handle_alarm_setting(void);
uint8_t* time_set_UI_HELPER(uint8_t* saved_alarmP);
void update_alarm_playlist(void);
void handle_alarm_playing(void);
void alarm_loop(void);
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

//  enable_oscillator_1(&hi2c1);
//  HAL_Delay(1000);
//
//  set_rtc_to_current_1(&hi2c1, 0, 12, 14, 4, 19, 11, 26);
//  HAL_Delay(1000);
  // to enable rtc module
//  uint8_t buff[1] = {0};
//   HAL_I2C_Mem_Write(&hi2c1, TIMER_ADDR, REG_TIME, I2C_MEMADD_SIZE_8BIT, buff, 1, 100);


  start_t = get_current_time_1(&hi2c1);
  uint8_t start_minute = start_t.minute;
  uint8_t start_hour = start_t.hour;
  uint8_t start_seconds = start_t.second;
  uint32_t start_time_in_sec = start_hour * 3600 + start_minute * 60 + start_seconds;
  today_time_in_sec = start_time_in_sec;



  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)value_adc, 2);

  // turn off built in led
  HAL_GPIO_WritePin(MY_LED_GPIO_Port, MY_LED_Pin, 1);

  update_alarm_playlist();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // ################################################################################################
	  // calculating today's current time in seconds
	  downtime_seconds = HAL_GetTick()/1000;
//	  today_time_in_sec = start_time_in_sec + downtime_seconds;

	  if (downtime_seconds - downtime_seconds_prev_HELPER >= 1){
		  today_time_in_sec += 1;
		  downtime_seconds_prev_HELPER = downtime_seconds;
		  handle_alarm_playing();
	  }

	  if (today_time_in_sec > (86400)){
		  today_time_in_sec = get_current_time_in_sec_1(&hi2c1);
	  }
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

	  else if (current_layer == 15){
		  handle_stop_watch();
	  }

	  else if (current_layer == 20){
		  handle_alarm_setting();
		  update_alarm_playlist();
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
  HAL_GPIO_WritePin(GPIOA, TM_CLK_Pin|TM_DIO_Pin, GPIO_PIN_RESET);

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
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
		single_quick_switch_counter = 0;
		current_layer = 2;  // Entering to particular function
		if (current_function == 2){
			current_layer = 15;
			auto_display_off_ENABLE = false;
		}

		else if (current_function == 0){
			current_layer = 20;
			auto_display_off_ENABLE = false;
		}
	}
	else if (double_quick_switch_counter && (current_layer <= 2)){
		current_layer -= 1;
	}
	// timer option selected
	else if (current_layer == 2 && single_quick_switch_counter){
		single_quick_switch_counter = 0;
		if (current_function == 1){
			current_layer = 10; // specific for timer
			auto_display_off_ENABLE = true;
		}
//		else if (current_function == 2){
//			current_layer = 15;  // specific for stop watch
//			auto_display_off_ENABLE = false;
//		}
	}
	// timer is finished, stop beeep-beep
	else if (current_layer == 10 && single_quick_switch_counter && start_beep_beep_FLAG){
		current_layer = 0;
		stop_beep_beep_FLAG = true;
		auto_display_off_ENABLE = true;
	}
	// cancel the timer with long_switch press
	else if (current_layer == 10 && long_switch_pressed && !start_beep_beep_FLAG){
		current_layer = 0;
		auto_display_off_ENABLE = true;
	}
	// cancel the stop watch
	else if(current_layer == 15 && double_quick_switch_counter){
		if (stop_watch_layer == 0){
			current_layer = 0;
			auto_display_off_ENABLE = true;
		}
	}

	if (current_layer < 0) current_layer = 0;
}


void handle_stop_watch(void){
	current_sw_tick = HAL_GetTick();
	if (stop_watch_layer == 0 && stop_watch_zero_display_FLAG){
		// reset the buffer with zero
		for (uint8_t i = 0; i < 4; i++){
			stop_w_data[i] = 0;
		}
		display_all_digits(stop_w_data);
		stop_watch_zero_display_FLAG = false;  // so that only one time 00:00 sets
	}

	// start counting
	if (stop_watch_layer == 0 && single_quick_switch_counter){
		single_quick_switch_counter = 0;
		prev_sw_tick = current_sw_tick;
		stop_watch_layer = 1;
		sw_counter = 0;
	}

	// counting
	else if(stop_watch_layer == 1){

		if (single_quick_switch_counter){
			single_quick_switch_counter = 0;
			stop_watch_layer = 2;
			return;
		}

		if (current_sw_tick - prev_sw_tick >= 1000){
			// display the count
			sw_counter += 1;
			uint8_t sw_minutes = (uint8_t)(sw_counter / 60);
			uint8_t sw_seconds = (uint8_t)(sw_counter - sw_minutes*60);
			stop_w_data[0] = sw_minutes / 10;
			stop_w_data[1] = sw_minutes % 10;
			stop_w_data[2] = sw_seconds / 10;
			stop_w_data[3] = sw_seconds % 10;
			display_all_digits(stop_w_data);
			prev_sw_tick = current_sw_tick;
		}
	}

	// continue
	else if(stop_watch_layer == 2 && single_quick_switch_counter){
		single_quick_switch_counter = 0;
		stop_watch_layer = 1;
		return;
	}

	// reset
	else if(stop_watch_layer == 2 && long_switch_pressed){
		long_switch_pressed = 0;
		stop_watch_layer = 0;
		stop_watch_zero_display_FLAG = true;
	}
}



void update_alarm_playlist(void){
	// extract saved alarms from memory
	uint16_t memory_addr = 0x0000u;  // address for page1's first byte
	total_enabled_alarms = 0;
	for (uint8_t i = 0; i < 4; i++){
		uint8_t rData[6] = {0,0,0,0,0,0};
		if(at24_isConnected(&hi2c1)){
		  	at24_read(&hi2c1, memory_addr, rData, 5, 100);
		  	HAL_Delay(10);

			// only get alarm which are enabled
		  	if (rData[4] == 1){
		  		my_alarm_playlist[total_enabled_alarms] = (uint32_t)(rData[0]*10 + rData[1])*3600UL + (uint32_t)(rData[2]*10 + rData[3])*60UL;
		  		total_enabled_alarms += 1;
		  	}
		}
		memory_addr += 64;
	}

	// sort my_alarm_playlist in ascending order
	for (uint8_t i = 1; i < total_enabled_alarms; i++){
		uint32_t key = my_alarm_playlist[i];
		int8_t j = i - 1;
		while (j >= 0 && my_alarm_playlist[j] > key){
			my_alarm_playlist[j + 1] = my_alarm_playlist[j];
			j--;
		}
		my_alarm_playlist[j + 1] = key;
	}
}




void handle_alarm_playing(void){
	// my_alarm_playlist will sorted
	for(uint8_t i=0; i < total_enabled_alarms; i++){
		// compare it with saved alarms and trigger alarm if any match
		if (my_alarm_playlist[i] == today_time_in_sec) {
			alarm_loop(); // this is blocking function
			today_time_in_sec = get_current_time_in_sec_1(&hi2c1);  // time update
		}

	}

}

// single_quick_switch means stops alarm and any joystick command for more than 2 second as snooze
// when exiting loop update the today_time_in_sec
void alarm_loop(void){
	uint16_t alarm_tone[4] = {150, 100, 150, 800};  // play, stop, play, stop
	uint8_t array_len = sizeof(alarm_tone)/sizeof(alarm_tone[0]);  // because size of returns total bytes
	uint8_t current_index = 0; // index of alarm_tone array
	uint32_t index_counter = 0;
	int joy_comm__ = JOY_IDLE, prev_joy_comm__ = JOY_IDLE;
	uint32_t alarm_start_time_ = today_time_in_sec;
	uint32_t current_time_ = alarm_start_time_;
	uint32_t current_tick;
	uint32_t snooze_alarm_tick_helper1, snooze_alarm_helper2;
	uint32_t alarm_helper1 = HAL_GetTick();
	bool flag1 = true; // used to enter the if statement which plays or stops speaker

	bool snooze_mode_enterd = false;

	char display_data1[4] = {'A', 'A', 'A', 'A'};
	char display_data2[4] = {'z', 'z', 'z', 'z'};

	while(true){
		current_tick = HAL_GetTick();
		detect_switch_cases_v2();
		joy_comm__ = get_joystick_commands_continuous();

		// update the loop time in sec
		current_time_ = current_tick/1000;


		// stop the alarm
		if (single_quick_switch_counter) {
			single_quick_switch_counter = 0;
			speaker_stop(&htim2);
			HAL_Delay(20);
			break;
		}
		// snooze the alarm
		if (joy_comm__ != JOY_IDLE){
			if (joy_comm__ != prev_joy_comm__){
				snooze_alarm_tick_helper1 = current_tick;
			}

			// if more than 2 second than enter to snooze mode
			if ((current_tick - snooze_alarm_tick_helper1 > 2000) && !snooze_mode_enterd){
				snooze_mode_enterd = true;
				snooze_alarm_helper2 = current_time_;  // will be used to compare 5 minutes for again trigger alarm
				speaker_stop(&htim2);
				HAL_Delay(20);
				display_all_chars(display_data2);
			}

		}

		// this is where alarm tone will be played
		if (!snooze_mode_enterd){
			if (flag1){
				alarm_helper1 = current_tick;
				flag1 = false;
				if (current_index%2 == 0){
					speaker_play(&htim2, 350);
					display_all_chars(display_data1);
				}
				else{
					speaker_stop(&htim2);
					display_all_chars(display_data2);
				}
			}

			// increment current_index
			if(current_tick - alarm_helper1 >= alarm_tone[current_index]){
				flag1 = true;
				index_counter += 1;
				current_index = index_counter % array_len;
			}
		}
		else{
			// 300 seconds = 5 minutes
			if(current_time_ - snooze_alarm_helper2 >= 300){
				snooze_mode_enterd = false;
			}
		}

		prev_joy_comm__ = joy_comm__;
	}

	display_all_chars(display_data2);
}






void handle_alarm_setting(void){
	bool run_loop = true;
	uint8_t alarm_layer = 0;  // 0: all saved alarm can be scrolled by up and down, 1: set or notSet the selected alarm, 2: UI for setting alarm
	int joy_comm;
	int prev_joy_comm = JOY_IDLE;
	int8_t alarm_number = 0;
	bool is_alarm_enable = false; // alarm is disabled
//	uint8_t display_numbers[4];
	char display_characters[4];

	// extract saved alarms from memory
	static uint8_t my_saved_alarms[4][5];
	uint16_t memory_addr = 0x0000u;  // address for page1's first byte
	for (uint8_t i = 0; i < 4; i++){
		uint8_t rData[6] = {0,0,0,0,0,0};
		if(at24_isConnected(&hi2c1)){
		  	at24_read(&hi2c1, memory_addr, rData, 5, 100);
		  	HAL_Delay(10);
		}
		// Copy 5 bytes from rData into row 'i' of my_saved_alarms
		memcpy(my_saved_alarms[i], rData, 5);
		memory_addr += 64;
	}
	memory_addr = 0x0000u; // reset

	while (run_loop){
		detect_switch_cases_v2();
		joy_comm = get_joystick_commands();

		if (alarm_layer == 0 && double_quick_switch_counter){
			current_layer = 0;
			auto_display_off_ENABLE = true;
			run_loop = false;
			// update the current time
			today_time_in_sec = get_current_time_in_sec_1(&hi2c1);
			break;
		}

		// upon selecting particular indexed alarm
		if(alarm_layer == 0 && single_quick_switch_counter){
			single_quick_switch_counter = 0;
			alarm_layer = 1;
			is_alarm_enable = my_saved_alarms[alarm_number][4];
			if (is_alarm_enable){
				display_characters[0] = 'S';
				display_characters[1] = 'E';
				display_characters[2] = 't';
				display_characters[3] = ' ';
			}
			else{
				display_characters[0] = 'n';
				display_characters[1] = 'S';
				display_characters[2] = 'E';
				display_characters[3] = 't';
			}
			display_all_chars(display_characters);
		}

		if (alarm_layer == 1 && single_quick_switch_counter){
			single_quick_switch_counter = 0;
			if (!is_alarm_enable){
				alarm_layer = 0;
				uint8_t temp_enable = is_alarm_enable ? 1 : 0;
				if(at24_isConnected(&hi2c1)){
					at24_write(&hi2c1, (memory_addr + alarm_number*64 + 4), &temp_enable, 1, 100);
					HAL_Delay(10);
				}
			  	display_all_digits(my_saved_alarms[alarm_number]);
			  	HAL_Delay(10);
			  	// save the status
			  	my_saved_alarms[alarm_number][4] = temp_enable;
			}
			else{
				uint8_t *alarm_time_HELPER = time_set_UI_HELPER(&my_saved_alarms[alarm_number][0]);
				memcpy(my_saved_alarms[alarm_number], alarm_time_HELPER, 4);
				my_saved_alarms[alarm_number][4] = 1;
				alarm_layer = 0;
				// now save this set alarm to memory
				if(at24_isConnected(&hi2c1)){
					at24_write(&hi2c1, (memory_addr + alarm_number*64), my_saved_alarms[alarm_number], 5, 100);
					HAL_Delay(10);
				}

			}
		}


		if ((joy_comm != prev_joy_comm) && (joy_comm != JOY_IDLE)){
			if (alarm_layer == 0){
				if (joy_comm == JOY_U) alarm_number++;
				else if (joy_comm == JOY_D) alarm_number--;
				if (alarm_number > 3) alarm_number = 3;
				if (alarm_number < 0) alarm_number = 0;

				// display the current indexed alarm
				display_all_digits(my_saved_alarms[alarm_number]);

			}

			else if (alarm_layer == 1){
				if ((joy_comm == JOY_R) || (joy_comm == JOY_L)){
					is_alarm_enable = !is_alarm_enable;
					if (is_alarm_enable){
						display_characters[0] = 'S';
						display_characters[1] = 'E';
						display_characters[2] = 't';
						display_characters[3] = ' ';
					}
					else{
						display_characters[0] = 'n';
						display_characters[1] = 'S';
						display_characters[2] = 'E';
						display_characters[3] = 't';
					}
					display_all_chars(display_characters);
				}
			}
		}





		prev_joy_comm = joy_comm;
	}
}

uint8_t* time_set_UI_HELPER(uint8_t* saved_alarmP){
	// first time display data means what time to show when this function is called
	// I think it should be saved alarm
	uint8_t saved_alarm[4];
	memcpy(saved_alarm, saved_alarmP, 4);
	int8_t hours = saved_alarm[0]*10 + saved_alarm[1];
	int8_t minutes = saved_alarm[2]*10 + saved_alarm[3];
	static uint8_t display_numbers[4];  // made static for this local variable
	uint8_t cursor_pointer = 0; // 0 means hours and 1 means minutes
	bool show_time = true;
	int joy_comm_;
	int prev_joy_comm_ = JOY_IDLE;
	uint32_t current_tick;
	uint32_t prev_tick = 0;
	bool enable_single_increment = false;
	bool enable_fast_multiple_increment = false;
	uint32_t counter_TICK_HELPER = 0, counter_TICK_HELPER_1 = 0;

	// start to blink at two of the hour's digits
	while(true){
		current_tick = HAL_GetTick();
		detect_switch_cases_v2();
		joy_comm_ = get_joystick_commands_continuous();

		// detect single switch press as final time and exit the loop
		if (single_quick_switch_counter){
			single_quick_switch_counter = 0;
			// so that it always shows correct set time
			display_numbers[0] = hours / 10;
			display_numbers[1] = hours % 10;
			display_numbers[2] = minutes / 10;
			display_numbers[3] = minutes % 10;
			display_all_digits(display_numbers);
			break;
		}

		// for changing pointer
		if (joy_comm_ != prev_joy_comm_ && (joy_comm_ != JOY_IDLE)){
			if (joy_comm_ == JOY_R || (joy_comm_ == JOY_L)){
				cursor_pointer = !cursor_pointer;
			}
		}

		// detect quick vs long joy commands
		if (joy_comm_ == JOY_U || joy_comm_ == JOY_D) {
		    if (joy_comm_ != prev_joy_comm_) {
		        enable_single_increment = true;
		        enable_fast_multiple_increment = false;
		        counter_TICK_HELPER = current_tick;
		    } else if ((current_tick - counter_TICK_HELPER) > 800) {
		        enable_fast_multiple_increment = true;
		    }
		} else {
		    enable_fast_multiple_increment = false;
		}

		// now do action
		if (enable_single_increment){
			enable_single_increment = false;
			if (cursor_pointer == 0){
				if (joy_comm_ == JOY_U) hours += 1;
				else if (joy_comm_ == JOY_D) hours -= 1;

				// limiting the values
				if (hours > 23) hours = 0;
				else if (hours < 0) hours = 23;
			}
			else{
				if (joy_comm_ == JOY_U) minutes += 1;
				else if (joy_comm_ == JOY_D) minutes -= 1;

				// limiting the values
				if (minutes > 59) minutes = 0;
				else if (minutes < 0) minutes = 59;
			}
		}

		if (enable_fast_multiple_increment){
			show_time = true;
			if (current_tick - counter_TICK_HELPER_1 > 200){
				counter_TICK_HELPER_1 = current_tick;

				if (cursor_pointer == 0){
					if (joy_comm_ == JOY_U) hours += 1;
					else if (joy_comm_ == JOY_D) hours -= 1;

					// limiting the values
					if (hours > 23) hours = 0;
					else if (hours < 0) hours = 23;
				}
				else{
					if (joy_comm_ == JOY_U) minutes += 1;
					else if (joy_comm_ == JOY_D) minutes -= 1;

					// limiting the values
					if (minutes > 59) minutes = 0;
					else if (minutes < 0) minutes = 59;
				}
			}
		}

		// let's convert time to display data
		display_numbers[0] = hours / 10;
		display_numbers[1] = hours % 10;
		display_numbers[2] = minutes / 10;
		display_numbers[3] = minutes % 10;



		// blink at current cursor point to
		if (!show_time){
			if (cursor_pointer == 0){
				display_numbers[0] = 10; // means default which is no display
				display_numbers[1] = 10;
			}
			else{
				display_numbers[2] = 10;
				display_numbers[3] = 10;
			}
		}
		if (current_tick - prev_tick > 200){
			show_time = !show_time;

			// display the numbers
			display_all_digits(display_numbers);
			prev_tick = current_tick;
		}




		prev_joy_comm_ = joy_comm_;

		HAL_Delay(10);
	}

	display_numbers[0] = hours / 10;
	display_numbers[1] = hours % 10;
	display_numbers[2] = minutes / 10;
	display_numbers[3] = minutes % 10;

	return display_numbers;
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
//	uint8_t current_seconds = (uint8_t)(today_time_in_sec - current_hour*3600 - current_minute*60);


	time_display_data[0] = (current_hour / 10);
	time_display_data[1] = current_hour % 10;
	time_display_data[2] = (current_minute / 10);
	time_display_data[3] = current_minute % 10;


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
		if (value_adc[1] > 3500) {
			joystick_at_idle = false;
			return JOY_U;
		}
		// D
		if (value_adc[1] < 500){
			joystick_at_idle = false;
			return JOY_D;
		}




		// R
		if (value_adc[0] > 3500) {
			joystick_at_idle = false;
			return JOY_R;
		}
		// L
		if (value_adc[0] < 500){
			joystick_at_idle = false;
			return JOY_L;
		}


	}
	return 1000;  // just a random value other than [0:3]
}

int get_joystick_commands_continuous(void) {
    static int current_state = JOY_IDLE;

    // Return to IDLE once the stick is back in the center deadzone
    if ((value_adc[0] > 1000 && value_adc[0] < 3000) &&
        (value_adc[1] > 1000 && value_adc[1] < 3000)) {
        current_state = JOY_IDLE;
    }
    // Update direction when pushed past the outer thresholds
    else if (value_adc[1] > 3500) {
        current_state = JOY_U;
    } else if (value_adc[1] < 500) {
        current_state = JOY_D;
    } else if (value_adc[0] > 3500) {
        current_state = JOY_R;
    } else if (value_adc[0] < 500) {
        current_state = JOY_L;
    }

    return current_state;
}




void start_timer(uint8_t option){
	if (!start_beep_beep_FLAG){
	if (!start_timer_FLAG){
		switch (option){
		case 0:
			timer_seconds = 10*60;  // 10 minutes
			break;
		case 1:
			timer_seconds = 60*25; // 25 minutes
			break;
		case 2:
			timer_seconds = 60*45; // 45 minutes
			break;
		case 3:
			timer_seconds = 60*60; // 60 minutes
			break;
		}
		start_timer_FLAG = true;
		timer_FUTURE_time = today_time_in_sec + timer_seconds;

		// FIX 2: Initialize PREV_time so the first second doesn't tick prematurely
		timer_PREV_time = today_time_in_sec;
	}
	// start counting timer until 00:00
	else{
//		timer_CURRENT_time = HAL_GetTick()/1000;
		if (today_time_in_sec < timer_FUTURE_time){
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
