/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UP_KEY  65
#define DOWN_KEY 66
#define RIGHT_KEY 67
#define LEFT_KEY  68
#define SEL_KEY  13

#define LONG_CLICK_MIN 20
#define LONG_CLICK_MAX 50
#define LONG_CLICK_COUNT 30

#define DOUBLE_CLICK_MIN 100
#define DOUBLE_CLICK_MAX 200

#define NORMAL_CLICK_MIN 500

enum CLOCK_MODE{
	NORMAL_STATE,
	TIME_SETTING,
	ALARM_TIME_SETTING,
	MUSIC_SELECT
};

enum CLOCK_BUTTON{
	NO_KEY,
	UP,
	DOWN,
	RIGHT,
	LEFT,
	SEL
};

//설정모드 자리 나태니는 열거형
enum SETTING{
	NOT,
	AP,
	SET_H,
	SET_M,
	SET_S
};
struct clock_state{
	enum CLOCK_MODE mode;
	enum CLOCK_BUTTON button;
};

struct clock_state current_state;
enum SETTING setting = AP;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void time_display(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile int second, minute, hour;
volatile int AL_second, AL_minute, AL_hour;
volatile int timer_count;
char line[2][18], time_str[16];
uint8_t key_value;
char uart_buf[40];

char rcv_byte;
uint32_t last_time,current_time;
uint32_t time_interval;
uint32_t long_key_count;
uint8_t key_value;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance==TIM2)
  {
    if((timer_count%100)==0)
    {
      second++;

      if(second>=60)
      {
        minute++;
        second = 0;
      }
      else if(second < 0) second = 0;
      if(minute>=60)
      {
        hour++;
        minute = 0;
      }
      else if(minute < 0) minute = 0;
      if(hour>=24 || hour < 0)
      {
        hour = 0;
      }

      time_display();
    }
	timer_count++;
  }
}

void time_display(void)
{
	switch(current_state.mode)
	{
	case NORMAL_STATE:
		memset(line,0,sizeof(line));
		sprintf(line[0],"Korea Polytech  \r\n");
		if(hour>=12)
		{
		  strncpy(line[1],"PM ",3);
		  sprintf(time_str,"%02d:%02d:%02d\r\n",hour-12,minute,second);
		}
		else
		{
		  strncpy(line[1],"AM ",3);
		  sprintf(time_str,"%02d:%02d:%02d\r\n",hour,minute,second);
		}
		strcat(line[1],time_str);
		break;
	case TIME_SETTING:
		memset(line,0,sizeof(line));
		sprintf(line[0],"Korea Polytech  \r\n");
		//설정모드 자리가 어디 선택 되어 있는 가에 따라 깜빡임 다른 모습으로 출력
		switch(setting)
		{
		case AP:
			if(timer_count % 200 == 0)
			{
				if(hour>=12)
				{
				  strncpy(line[1],"   ",3);
				  sprintf(time_str,"%02d:%02d:%02d\r\n",hour-12,minute,second);
				}
				else
				{
				  strncpy(line[1],"   ",3);
				  sprintf(time_str,"%02d:%02d:%02d\r\n",hour,minute,second);
				}
				strcat(line[1],time_str);

			}
			else
			{
				if(hour>=12)
				{
				  strncpy(line[1],"PM ",3);
				  sprintf(time_str,"%02d:%02d:%02d\r\n",hour-12,minute,second);
				}
				else
				{
				  strncpy(line[1],"AM ",3);
				  sprintf(time_str,"%02d:%02d:%02d\r\n",hour,minute,second);
				}
				strcat(line[1],time_str);

			}
			break;
		case SET_H:
			if(timer_count % 200 == 0)
			{
				if(hour>=12)
				{
				  strncpy(line[1],"PM ",3);
				  sprintf(time_str,"  :%02d:%02d\r\n",minute,second);
				}
				else
				{
				  strncpy(line[1],"AM ",3);
				  sprintf(time_str,"  :%02d:%02d\r\n",minute,second);
				}
				strcat(line[1],time_str);

			}
			else
			{
				if(hour>=12)
				{
				  strncpy(line[1],"PM ",3);
				  sprintf(time_str,"%02d:%02d:%02d\r\n",hour-12,minute,second);
				}
				else
				{
				  strncpy(line[1],"AM ",3);
				  sprintf(time_str,"%02d:%02d:%02d\r\n",hour,minute,second);
				}
				strcat(line[1],time_str);

			}
			break;
		case SET_M:
			if(timer_count % 200 == 0)
			{
				if(hour>=12)
				{
				  strncpy(line[1],"PM ",3);
				  sprintf(time_str,"%02d:  :%02d\r\n",hour-12,second);
				}
				else
				{
				  strncpy(line[1],"AM ",3);
				  sprintf(time_str,"%02d:  :%02d\r\n",hour,second);
				}
				strcat(line[1],time_str);

			}
			else
			{
				if(hour>=12)
				{
				  strncpy(line[1],"PM ",3);
				  sprintf(time_str,"%02d:%02d:%02d\r\n",hour-12,minute,second);
				}
				else
				{
				  strncpy(line[1],"AM ",3);
				  sprintf(time_str,"%02d:%02d:%02d\r\n",hour,minute,second);
				}
				strcat(line[1],time_str);

			}
			break;
		case SET_S:
			if(timer_count % 200 == 0)
			{
				if(hour>=12)
				{
				  strncpy(line[1],"PM ",3);
				  sprintf(time_str,"%02d:%02d:  \r\n",hour-12,minute);
				}
				else
				{
				  strncpy(line[1],"AM ",3);
				  sprintf(time_str,"%02d:%02d:  \r\n",hour,minute);
				}
				strcat(line[1],time_str);

			}
			else
			{
				if(hour>=12)
				{
				  strncpy(line[1],"PM ",3);
				  sprintf(time_str,"%02d:%02d:%02d\r\n",hour-12,minute,second);
				}
				else
				{
				  strncpy(line[1],"AM ",3);
				  sprintf(time_str,"%02d:%02d:%02d\r\n",hour,minute,second);
				}
				strcat(line[1],time_str);

			}
			break;
		default:
			break;
		}
		break;
	case ALARM_TIME_SETTING:
		memset(line,0,sizeof(line));
		sprintf(line[0],"Korea Polytech  \r\n");
		//설정모드 자리가 어디 선택 되어 있는 가에 따라 깜빡임 다른 모습으로 출력
		switch(setting)
		{
		case AP:
			if(timer_count % 200 == 0)
			{
				if(AL_hour>=12)
				{
				  strncpy(line[1],"   ",3);
				  sprintf(time_str,"%02d:%02d:%02d AL\r\n",AL_hour-12,AL_minute,AL_second);
				}
				else
				{
				  strncpy(line[1],"   ",3);
				  sprintf(time_str,"%02d:%02d:%02d AL\r\n",AL_hour,AL_minute,AL_second);
				}
				strcat(line[1],time_str);

			}
			else
			{
				if(AL_hour>=12)
				{
				  strncpy(line[1],"PM ",3);
				  sprintf(time_str,"%02d:%02d:%02d AL\r\n",AL_hour-12,AL_minute,AL_second);
				}
				else
				{
				  strncpy(line[1],"AM ",3);
				  sprintf(time_str,"%02d:%02d:%02d AL\r\n",AL_hour,AL_minute,AL_second);
				}
				strcat(line[1],time_str);

			}
			break;
		case SET_H:
			if(timer_count % 200 == 0)
			{
				if(AL_hour>=12)
				{
				  strncpy(line[1],"PM ",3);
				  sprintf(time_str,"  :%02d:%02d AL\r\n",AL_minute,AL_second);
				}
				else
				{
				  strncpy(line[1],"AM ",3);
				  sprintf(time_str,"  :%02d:%02d AL\r\n",AL_minute,AL_second);
				}
				strcat(line[1],time_str);

			}
			else
			{
				if(AL_hour>=12)
				{
				  strncpy(line[1],"PM ",3);
				  sprintf(time_str,"%02d:%02d:%02d AL\r\n",AL_hour-12,AL_minute,AL_second);
				}
				else
				{
				  strncpy(line[1],"AM ",3);
				  sprintf(time_str,"%02d:%02d:%02d AL\r\n",AL_hour,AL_minute,AL_second);
				}
				strcat(line[1],time_str);

			}
			break;
		case SET_M:
			if(timer_count % 200 == 0)
			{
				if(AL_hour>=12)
				{
				  strncpy(line[1],"PM ",3);
				  sprintf(time_str,"%02d:  :%02d AL\r\n",AL_hour-12,AL_second);
				}
				else
				{
				  strncpy(line[1],"AM ",3);
				  sprintf(time_str,"%02d:  :%02d AL\r\n",AL_hour,AL_second);
				}
				strcat(line[1],time_str);

			}
			else
			{
				if(AL_hour>=12)
				{
				  strncpy(line[1],"PM ",3);
				  sprintf(time_str,"%02d:%02d:%02d AL\r\n",AL_hour-12,AL_minute,AL_second);
				}
				else
				{
				  strncpy(line[1],"AM ",3);
				  sprintf(time_str,"%02d:%02d:%02d AL\r\n",AL_hour,AL_minute,AL_second);
				}
				strcat(line[1],time_str);

			}
			break;
		case SET_S:
			if(timer_count % 200 == 0)
			{
				if(AL_hour>=12)
				{
				  strncpy(line[1],"PM ",3);
				  sprintf(time_str,"%02d:%02d:   AL\r\n",AL_hour-12,AL_minute);
				}
				else
				{
				  strncpy(line[1],"AM ",3);
				  sprintf(time_str,"%02d:%02d:   AL\r\n",AL_hour,AL_minute);
				}
				strcat(line[1],time_str);

			}
			else
			{
				if(AL_hour>=12)
				{
				  strncpy(line[1],"PM ",3);
				  sprintf(time_str,"%02d:%02d:%02d AL\r\n",AL_hour-12,AL_minute,AL_second);
				}
				else
				{
				  strncpy(line[1],"AM ",3);
				  sprintf(time_str,"%02d:%02d:%02d AL\r\n",AL_hour,AL_minute,AL_second);
				}
				strcat(line[1],time_str);

			}
			break;
		default:
			break;
		}
		break;
	case MUSIC_SELECT:
		memset(line,0,sizeof(line));
		sprintf(line[0],"Three Bears     \r\n");
		sprintf(line[1],"Spring Water    \r\n");
		break;
	default:
		break;
	}


  HAL_UART_Transmit_IT(&huart3,(uint8_t *)line,sizeof(line));
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART3)
  {

	current_time = HAL_GetTick();
    time_interval = current_time - last_time;
    last_time = current_time;

    switch(current_state.mode)
    {
      case NORMAL_STATE:
    	  switch(key_value)
    	  {
    	  	  case SEL_KEY:
        		  current_state.button = SEL;
        		  mode_analysis();
    	  		  break;
    	  	  default:  // Arrow keys is Not valid in NORMAL state
    	  		  break;
    	  }
    	  break;
      case TIME_SETTING:
    	  switch(key_value)
    	  {
    	  	  case SEL_KEY:
    	  		  if(time_interval>= NORMAL_CLICK_MIN)
    	  		  {
    	  			  setting = AP;
    	  			  current_state.mode = NORMAL_STATE;
    	  		  }
    	  		  //더블클릭하면 처음 클릭 부분에서 타임세팅으로 넘어가기 때문에 여기서 음악선택모드를 판별
    	  		  //mode_analysis()에서 판별하면 타임세팅에서 더블클릭해야 음악선택모드로 넘어감
    	  		  else if(time_interval>=DOUBLE_CLICK_MIN && time_interval <=DOUBLE_CLICK_MAX)
    	  		  {
    	  			  current_state.mode = MUSIC_SELECT;
    	  			  long_key_count = 0;

    	  			  memset(uart_buf,0,sizeof(uart_buf));
    	  			  sprintf(uart_buf,"MUSIC_SELECT %d\r\n",time_interval);
    	  			  HAL_UART_Transmit_IT(&huart3,uart_buf,sizeof(uart_buf));

    	  	    }
    	  		  break;
    	  	  case UP_KEY:
    	  		  switch(setting)
    	  		  {
    	  		  case AP:
    	  			  hour += 12;
    	  			  break;
    	  		  case SET_H:
    	  			  hour++;
    	  			  break;
    	  		  case SET_M:
    	  			  minute++;
    	  			  break;
    	  		  case SET_S:
    	  			  second++;
    	  			  break;
    	  		  default:
    	  			  break;
    	  		  }
    	  		  break;
    	  	  case DOWN_KEY:
    	  		  switch(setting)
    	  		  {
    	  		  case AP:
    	  			  hour -= 12;
    	  			  break;
    	  		  case SET_H:
    	  			  hour--;
    	  			  break;
    	  		  case SET_M:
    	  			  minute--;
    	  			  break;
    	  		  case SET_S:
    	  			  second--;
    	  			  break;
    	  		  default:
    	  			  break;
    	  		  }
    	  		  break;
    	  	  case RIGHT_KEY:
    	  		  setting++;
    	  		  if(setting == 5) setting = AP;
    	  		  break;
    	  	  case LEFT_KEY:
    	  		  setting--;
    	  		  if(setting == NOT) setting = SET_S;
    	  		  break;
    	  	  default:  // Arrow keys is Not valid in NORMAL state
    	  		  break;
    	  }
    	  break;
      case ALARM_TIME_SETTING:
    	  switch(key_value)
    	  {
    	  	  case SEL_KEY:
    	  		  if(time_interval>= NORMAL_CLICK_MIN)
    	  		  {
    	  			  setting = AP;
    	  			  current_state.mode = NORMAL_STATE;
    	  		  }
    	  		  break;
    	  	  case UP_KEY:
    	  		  switch(setting)
    	  		  {
    	  		  case AP:
    	  			  AL_hour += 12;
    	  			  if(AL_hour > 12 || AL_hour <= 0) AL_hour = 0;
    	  			  break;
    	  		  case SET_H:
    	  			  AL_hour++;
    	  			  if(AL_hour > 12 || AL_hour <= 0) AL_hour = 0;
    	  			  break;
    	  		  case SET_M:
    	  			  AL_minute++;
    	  			  if(AL_minute > 60 || AL_minute <= 0) AL_minute = 0;
    	  			  break;
    	  		  case SET_S:
    	  			  AL_second++;
    	  			  if(AL_second > 60 || AL_second <= 0) AL_second = 0;
    	  			  break;
    	  		  default:
    	  			  break;
    	  		  }
    	  		  break;
    	  	  case DOWN_KEY:
    	  		  switch(setting)
    	  		  {
    	  		  case AP:
    	  			  AL_hour -= 12;
    	  			  if(AL_hour > 12 || AL_hour <= 0) AL_hour = 0;
    	  			  break;
    	  		  case SET_H:
    	  			  AL_hour--;
    	  			  if(AL_hour > 12 || AL_hour <= 0) AL_hour = 0;
    	  			  break;
    	  		  case SET_M:
    	  			  AL_minute--;
    	  			  if(AL_minute > 60 || AL_minute <= 0) AL_minute = 0;
    	  			  break;
    	  		  case SET_S:
    	  			  AL_second--;
    	  			  if(AL_second > 60 || AL_second <= 0) AL_second = 0;
    	  			  break;
    	  		  default:
    	  			  break;
    	  		  }
    	  		  break;
    	  	  case RIGHT_KEY:
    	  		  setting++;
    	  		  if(setting == 5) setting = AP;
    	  		  break;
    	  	  case LEFT_KEY:
    	  		  setting--;
    	  		  if(setting == NOT) setting = SET_S;
    	  		  break;
    	  	  default:  // Arrow keys is Not valid in NORMAL state
    	  		  break;
    	  }
    	  break;
      case MUSIC_SELECT:
    	  switch(key_value)
    	  {
	  	  	  case SEL_KEY:
	  	  		  if(time_interval>= NORMAL_CLICK_MIN) current_state.mode = NORMAL_STATE;
	  	  		  break;
	  	  	  case UP_KEY:
	  	  		  memset(uart_buf,0,sizeof(uart_buf));
	  	  		  sprintf(uart_buf,"Three Bears selected\r\n");
	  	  		  HAL_UART_Transmit_IT(&huart3,uart_buf,sizeof(uart_buf));
	  	  		  break;
	  	  	  case DOWN_KEY:
	  	  		  memset(uart_buf,0,sizeof(uart_buf));
	  	  		  sprintf(uart_buf,"Spring Water selected\r\n");
	  	  		  HAL_UART_Transmit_IT(&huart3,uart_buf,sizeof(uart_buf));
	  	  		  break;
    	  }
    	  break;
      default:
    	  break;
    }


	/* Receive one byte in interrupt mode */
	HAL_UART_Receive_IT(&huart3, &key_value, 1);
  }
}

//타임세팅, 알람세팅으로 넘어가는 여부 판별 함수 (음악선택은 제외)
void mode_analysis(void)
{
    if(time_interval>=LONG_CLICK_MIN && time_interval <=LONG_CLICK_MAX)
    {
    	long_key_count++;
    }
    else if(time_interval>= NORMAL_CLICK_MIN)
    {
      current_state.mode = TIME_SETTING;
      long_key_count = 0;

      memset(uart_buf,0,sizeof(uart_buf));
      sprintf(uart_buf,"TIME_SETTING  %d\r\n",time_interval);
      HAL_UART_Transmit_IT(&huart3,uart_buf,sizeof(uart_buf));

    }

    if(long_key_count>=LONG_CLICK_COUNT)
    {
      current_state.mode  = ALARM_TIME_SETTING;
      long_key_count = 0;

      memset(uart_buf,0,sizeof(uart_buf));
      sprintf(uart_buf,"ALARM_TIME_SETTING  %d\r\n",time_interval);
      HAL_UART_Transmit_IT(&huart3,uart_buf,sizeof(uart_buf));
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
  MX_USART3_UART_Init();
  MX_RTC_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Init(&htim2);
  HAL_TIM_Base_Start_IT(&htim2);

  HAL_UART_Receive_IT(&huart3,&rcv_byte,1);	//UART 수신 인터럽트 초기설정 이거 없으면 인터럽트 시작 안함
  
  current_state.mode = NORMAL_STATE;
  current_state.button = NO_KEY;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

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

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1000;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 900;
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
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_MDC_Pin RMII_RXD0_Pin RMII_RXD1_Pin */
  GPIO_InitStruct.Pin = RMII_MDC_Pin|RMII_RXD0_Pin|RMII_RXD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_REF_CLK_Pin RMII_MDIO_Pin RMII_CRS_DV_Pin */
  GPIO_InitStruct.Pin = RMII_REF_CLK_Pin|RMII_MDIO_Pin|RMII_CRS_DV_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : RMII_TXD1_Pin */
  GPIO_InitStruct.Pin = RMII_TXD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(RMII_TXD1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_SOF_Pin USB_ID_Pin USB_DM_Pin USB_DP_Pin */
  GPIO_InitStruct.Pin = USB_SOF_Pin|USB_ID_Pin|USB_DM_Pin|USB_DP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_VBUS_Pin */
  GPIO_InitStruct.Pin = USB_VBUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_VBUS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_TX_EN_Pin RMII_TXD0_Pin */
  GPIO_InitStruct.Pin = RMII_TX_EN_Pin|RMII_TXD0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
