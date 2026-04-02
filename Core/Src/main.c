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
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "ssd1306.h"
#include "fonts.h"
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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

float lower_t = 100;

float higher_t = 0;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define DEBUG_ENABLE 0

#if DEBUG_ENABLE
    #define DEBUG_PRINT(fmt, ...) printf("[DEBUG] %s:%d: " fmt "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
    #define DEBUG_INFO(fmt, ...)  printf("[INFO] " fmt "\r\n", ##__VA_ARGS__)
    #define DEBUG_ERR(fmt, ...)   printf("[ERROR] " fmt "\r\n", ##__VA_ARGS__)

	#ifdef __GNUC__
		#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
	#else
		#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
	#endif

	PUTCHAR_PROTOTYPE
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
		return ch;
	}

#else
    #define DEBUG_PRINT(fmt, ...)
    #define DEBUG_INFO(fmt, ...)
    #define DEBUG_ERR(fmt, ...)
#endif

void get_float_str(char *buf, float temp, char *pre_str, char *unit_str) {
    int32_t integral = (int32_t)temp;
    int32_t fractional = (int32_t)((temp - integral) * 100);
    if(fractional < 0) fractional = -fractional;

    // ???????,????
    sprintf(buf, "%d.%02d%s", integral, fractional, unit_str);
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
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

	DEBUG_PRINT("Initialize the thermometer\r\n");

	uint8_t channel = 0x2; // 0x01 ???? 0, 0x02 ???? 1...
	HAL_I2C_Master_Transmit(&hi2c1, 0x70 << 1, &channel, 1, 100);
	HAL_Delay(500);
	uint8_t reset_cmd = 0xFE;
	HAL_I2C_Master_Transmit(&hi2c1, 0x80, &reset_cmd, 1, 100);
	HAL_Delay(500);

	DEBUG_PRINT("Initialize the 0led\r\n");
	channel = 0x1; // 0x01 ???? 0, 0x02 ???? 1...
	HAL_I2C_Master_Transmit(&hi2c1, 0x70 << 1, &channel, 1, 100);
	HAL_Delay(500);

/*
	printf("\r\n--- I2C Bus Scanner Start ---\r\n");
  printf("Scanning channel %d\r\n", channel);
  printf("Scanning I2C1 (PB6:SCL, PB7:SDA)...\r\n");

  uint8_t count = 0;
  for (uint16_t i = 1; i < 128; i++) {
      if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(i << 1), 3, 10) == HAL_OK) {
          printf("Found Device! 7-bit: 0x%02X (8-bit: 0x%02X)\r\n", i, i << 1);
          count++;
      }
  }

  if (count == 0) {
      printf("No I2C devices found. Check:\r\n1. Wiring (PB6/PB7)\r\n2. Power (3.3V)\r\n3. Pull-up Resistors\r\n");
  } else {
      printf("Scan finished. Total %d device(s) found.\r\n", count);
  }
  printf("------------------------------\r\n\r\n");
*/

  // Init lcd using one of the stm32HAL i2c typedefs
  if (ssd1306_Init(&hi2c1) != 0) {
    Error_Handler();
  }
  HAL_Delay(500);

  ssd1306_Fill(Black);
  ssd1306_UpdateScreen(&hi2c1);

  HAL_Delay(500);

  // Write data to local screenbuffer
	ssd1306_SetCursor(0, 6);
  ssd1306_WriteString("Hello", Font_7x10, White);

  ssd1306_SetCursor(0, 25);
  ssd1306_WriteString("World", Font_7x10, White);

	ssd1306_UpdateScreen(&hi2c1);

  HAL_Delay(500);

  DEBUG_PRINT("OLED Displayed: Hello World!!!\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	char temperature_str[16];
	char lower_t_str[16];
	char higher_t_str[16];
	char humidity_str[16];
	bool dis_low = true;

  while (1)
  {
		channel = 0x2; // 0x01 ???? 0, 0x02 ???? 1...
		HAL_I2C_Master_Transmit(&hi2c1, 0x70 << 1, &channel, 1, 100);
		HAL_Delay(200);

		uint8_t cmd_temp = 0xF3; // ?????? (No Hold)
    uint8_t cmd_humi = 0xF5; // ?????? (No Hold)
    uint8_t data[3];         // ?? [?8?, ?8?, ???]
    float temperature, humidity;

    // --- 1. ???? ---
    if (HAL_I2C_Master_Transmit(&hi2c1, 0x80, &cmd_temp, 1, 100) == HAL_OK)
    {
        HAL_Delay(100); // ???? > 85ms (14-bit ????)

        if (HAL_I2C_Master_Receive(&hi2c1, 0x81, data, 3, 100) == HAL_OK)
        {
            // SHT2x ??????: T = -46.85 + 175.72 * (Raw / 2^16)
            uint16_t raw_t = (data[0] << 8) | data[1];
            raw_t &= ~0x0003; // ?????????
            temperature = -46.85f + 175.72f * ((float)raw_t / 65536.0f);
            get_float_str(temperature_str, temperature, "T", "C");
						if (lower_t > temperature) {
							lower_t = temperature;
							memcpy(lower_t_str, temperature_str, 16);
						}
						if (higher_t < temperature) {
							higher_t = temperature;
							memcpy(higher_t_str, temperature_str, 16);
						}
        }
    }

    // --- 2. ???? ---
    if (HAL_I2C_Master_Transmit(&hi2c1, 0x80, &cmd_humi, 1, 100) == HAL_OK)
    {
        HAL_Delay(100); // ?????? (12-bit ? 29ms)

        if (HAL_I2C_Master_Receive(&hi2c1, 0x81, data, 3, 100) == HAL_OK)
        {
            // SHT2x ??????: RH = -6 + 125 * (Raw / 2^16)
            uint16_t raw_h = (data[0] << 8) | data[1];
            raw_h &= ~0x0003; // ?????
            humidity = -6.0f + 125.0f * ((float)raw_h / 65536.0f);
            get_float_str(humidity_str, humidity, "H", "%");
        }
    }

    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // PC13 ????????
    HAL_Delay(200); // ??????

		channel = 0x1; // 0x01 ???? 0, 0x02 ???? 1...
		HAL_I2C_Master_Transmit(&hi2c1, 0x70 << 1, &channel, 1, 100);
		HAL_Delay(200);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		ssd1306_Fill(Black);

		ssd1306_SetCursor(0, 0);
		ssd1306_WriteString("Temp:", Font_7x10, White);
		ssd1306_SetCursor(0, 10);
		ssd1306_WriteString(temperature_str, Font_7x10, White);
		ssd1306_SetCursor(0, 20);
		ssd1306_WriteString("Humi:", Font_7x10, White);
		ssd1306_SetCursor(0, 30);
		ssd1306_WriteString(humidity_str, Font_7x10, White);
		if (dis_low) {
			ssd1306_SetCursor(0, 40);
			ssd1306_WriteString("LowT", Font_7x10, White);
			ssd1306_SetCursor(0, 50);
			ssd1306_WriteString(lower_t_str, Font_7x10, White);
			dis_low = false;
		} else {
			ssd1306_SetCursor(0, 40);
			ssd1306_WriteString("HighT", Font_7x10, White);
			ssd1306_SetCursor(0, 50);
			ssd1306_WriteString(higher_t_str, Font_7x10, White);
			dis_low = true;
		}
		ssd1306_UpdateScreen(&hi2c1);
		DEBUG_PRINT("Temp:%s Humi:%s\r\n", temperature_str, humidity_str);
		HAL_Delay(200);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
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
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;   // ????
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
