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
#include "epd_213_bwr.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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
SPI_HandleTypeDef hspi2;
TIM_HandleTypeDef htim6;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
volatile uint32_t g_tim6_ms = 0U;
static uint32_t g_last_btn_tick = 0U;
static uint8_t g_blink_enabled = 1U;
static uint32_t g_last_led_toggle_ms = 0U;
static uint32_t g_last_log_ms = 0U;
static volatile uint8_t g_btn_event_pending = 0U;
static char g_uart_cmd_buf[64];
static uint8_t g_uart_cmd_len = 0U;
static EPD213_HandleTypeDef g_epd;
static uint8_t g_epd_inited = 0U;
static uint8_t g_epd_auto_refresh_enabled = 1U;
static uint32_t g_epd_next_refresh_ms = 0U;
static uint32_t g_epd_refresh_count = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM6_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
static void App_Log(const char *fmt, ...);
static void App_ProcessSerialCmd(void);
static void App_HandleCommand(void);
static HAL_StatusTypeDef App_EpdEnsureInit(void);
static HAL_StatusTypeDef App_EpdScrollText(const char *text);
static void App_EpdAutoRefreshTask(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */


/* USER CODE BEGIN PD */
#define LED_GPIO_PORT GPIOC
#define LED_GPIO_PIN  GPIO_PIN_6
#define BTN_GPIO_PORT GPIOC
#define BTN_GPIO_PIN  GPIO_PIN_13
#define BTN_DEBOUNCE_MS 30U
#define UART_RX_CHAR_TIMEOUT_MS 0U
#define EPD_CS_GPIO_PORT GPIOB
#define EPD_CS_GPIO_PIN  GPIO_PIN_12
#define EPD_DC_GPIO_PORT GPIOB
#define EPD_DC_GPIO_PIN  GPIO_PIN_14
#define EPD_RES_GPIO_PORT GPIOA
#define EPD_RES_GPIO_PIN  GPIO_PIN_10
#define EPD_BUSY_GPIO_PORT GPIOA
#define EPD_BUSY_GPIO_PIN  GPIO_PIN_9
#define EPD_SCROLL_TEXT_X 4U
#define EPD_SCROLL_STEP_PX 24U
#define EPD_SCROLL_DELAY_MS 300U
#define EPD_TEXT_HEIGHT_PX 7U
#define EPD_AUTO_REFRESH_INTERVAL_MS 1000U
/* USER CODE END PD */

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
  MX_SPI2_Init();
  MX_TIM6_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  g_epd.hspi = &hspi2;
  g_epd.cs_port = EPD_CS_GPIO_PORT;
  g_epd.cs_pin = EPD_CS_GPIO_PIN;
  g_epd.dc_port = EPD_DC_GPIO_PORT;
  g_epd.dc_pin = EPD_DC_GPIO_PIN;
  g_epd.res_port = EPD_RES_GPIO_PORT;
  g_epd.res_pin = EPD_RES_GPIO_PIN;
  g_epd.busy_port = EPD_BUSY_GPIO_PORT;
  g_epd.busy_pin = EPD_BUSY_GPIO_PIN;

  if (HAL_TIM_Base_Start_IT(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  App_Log("\r\n=== USART2 log online ===\r\n");
  App_Log("PC13 button toggles LED blink\r\n");
  App_Log("Commands: on/off/toggle/status/help\r\n");
  App_Log("EPD: epdinit/epdclear/epdtest/epdbusy/epdsleep/epdtext/epdscroll\r\n");
  App_Log("EPD auto refresh=ON, interval=%lu ms\r\n", (unsigned long)EPD_AUTO_REFRESH_INTERVAL_MS);
  g_epd_next_refresh_ms = g_tim6_ms - EPD_AUTO_REFRESH_INTERVAL_MS;
  App_Log("> ");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    if (g_blink_enabled && ((g_tim6_ms - g_last_led_toggle_ms) >= 500U))
    {
      g_last_led_toggle_ms = g_tim6_ms;
      HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
    }
    if ((g_tim6_ms - g_last_log_ms) >= 1000U)
    {
      g_last_log_ms = g_tim6_ms;
      App_Log("[tick=%lu ms] blink=%s\r\n",
              (unsigned long)g_tim6_ms,
              g_blink_enabled ? "ON" : "OFF");
    }
    App_ProcessSerialCmd();
    App_EpdAutoRefreshTask();
    if (g_btn_event_pending != 0U)
    {
      g_btn_event_pending = 0U;
      App_Log("button event: blink=%s\r\n", g_blink_enabled ? "ON" : "OFF");
    }

    __WFI();

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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
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
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(EPD_CS_GPIO_PORT, EPD_CS_GPIO_PIN | EPD_DC_GPIO_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(EPD_RES_GPIO_PORT, EPD_RES_GPIO_PIN, GPIO_PIN_SET);

  /*Configure GPIO pin : PC6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = BTN_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(BTN_GPIO_PORT, &GPIO_InitStruct);

  /*Configure EPD control pins : PB12 CS, PB14 DC */
  GPIO_InitStruct.Pin = EPD_CS_GPIO_PIN | EPD_DC_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(EPD_CS_GPIO_PORT, &GPIO_InitStruct);

  /*Configure EPD reset pin : PA10 */
  GPIO_InitStruct.Pin = EPD_RES_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(EPD_RES_GPIO_PORT, &GPIO_InitStruct);

  /*Configure EPD busy pin : PA9 */
  GPIO_InitStruct.Pin = EPD_BUSY_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(EPD_BUSY_GPIO_PORT, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 17000 - 1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 10 - 1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
static void App_Log(const char *fmt, ...)
{
  char buf[128];
  va_list args;
  int n;

  va_start(args, fmt);
  n = vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if (n <= 0)
  {
    return;
  }
  if (n > (int)sizeof(buf))
  {
    n = (int)sizeof(buf);
  }

  (void)HAL_UART_Transmit(&huart2, (uint8_t *)buf, (uint16_t)n, 100U);
}

static void App_ProcessSerialCmd(void)
{
  uint8_t ch;

  if (HAL_UART_Receive(&huart2, &ch, 1U, UART_RX_CHAR_TIMEOUT_MS) != HAL_OK)
  {
    return;
  }

  if ((ch == '\r') || (ch == '\n'))
  {
    if (g_uart_cmd_len > 0U)
    {
      g_uart_cmd_buf[g_uart_cmd_len] = '\0';
      App_Log("\r\n");
      App_HandleCommand();
      g_uart_cmd_len = 0U;
    }
    else
    {
      App_Log("\r\n");
    }
    App_Log("> ");
    return;
  }

  if ((ch == '\b') || (ch == 0x7FU))
  {
    if (g_uart_cmd_len > 0U)
    {
      g_uart_cmd_len--;
      App_Log("\b \b");
    }
    return;
  }

  if ((ch < 32U) || (ch > 126U))
  {
    return;
  }

  if (g_uart_cmd_len >= (sizeof(g_uart_cmd_buf) - 1U))
  {
    g_uart_cmd_len = 0U;
    App_Log("\r\ncommand too long\r\n> ");
    return;
  }

  g_uart_cmd_buf[g_uart_cmd_len++] = (char)ch;
  (void)HAL_UART_Transmit(&huart2, &ch, 1U, 50U);
}

static void App_HandleCommand(void)
{
  HAL_StatusTypeDef epd_status;
  uint8_t i;

  for (i = 0U; i < g_uart_cmd_len; i++)
  {
    if (g_uart_cmd_buf[i] == ' ')
    {
      break;
    }
    if ((g_uart_cmd_buf[i] >= 'A') && (g_uart_cmd_buf[i] <= 'Z'))
    {
      g_uart_cmd_buf[i] = (char)(g_uart_cmd_buf[i] + ('a' - 'A'));
    }
  }

  if (strcmp(g_uart_cmd_buf, "on") == 0)
  {
    g_blink_enabled = 1U;
    App_Log("blink=ON\r\n");
  }
  else if (strcmp(g_uart_cmd_buf, "off") == 0)
  {
    g_blink_enabled = 0U;
    App_Log("blink=OFF\r\n");
  }
  else if (strcmp(g_uart_cmd_buf, "toggle") == 0)
  {
    g_blink_enabled ^= 1U;
    App_Log("blink=%s\r\n", g_blink_enabled ? "ON" : "OFF");
  }
  else if (strcmp(g_uart_cmd_buf, "status") == 0)
  {
    App_Log("status: tick=%lu ms, blink=%s\r\n",
            (unsigned long)g_tim6_ms,
            g_blink_enabled ? "ON" : "OFF");
  }
  else if (strcmp(g_uart_cmd_buf, "help") == 0)
  {
    App_Log("commands: on | off | toggle | status | help\r\n");
    App_Log("epd: epdinit | epdclear | epdtest | epdbusy | epdsleep\r\n");
    App_Log("epd: epdtext <msg> | epdscroll [msg]\r\n");
    App_Log("epd: epdautoon | epdautooff\r\n");
  }
  else if (strcmp(g_uart_cmd_buf, "epdbusy") == 0)
  {
    App_Log("epd busy=%u\r\n", (unsigned int)EPD213_IsBusy(&g_epd));
  }
  else if (strcmp(g_uart_cmd_buf, "epdinit") == 0)
  {
    epd_status = EPD213_Init(&g_epd);
    if (epd_status == HAL_OK)
    {
      g_epd_inited = 1U;
      App_Log("epd init ok\r\n");
    }
    else
    {
      g_epd_inited = 0U;
      App_Log("epd init fail: %lu\r\n", (unsigned long)epd_status);
    }
  }
  else if (strcmp(g_uart_cmd_buf, "epdclear") == 0)
  {
    epd_status = App_EpdEnsureInit();
    if (epd_status != HAL_OK)
    {
      App_Log("epd not ready: %lu\r\n", (unsigned long)epd_status);
      return;
    }
    epd_status = EPD213_Clear(&g_epd);
    if (epd_status == HAL_OK)
    {
      App_Log("epd clear ok\r\n");
    }
    else
    {
      App_Log("epd clear fail: %lu\r\n", (unsigned long)epd_status);
    }
  }
  else if (strcmp(g_uart_cmd_buf, "epdtest") == 0)
  {
    epd_status = App_EpdEnsureInit();
    if (epd_status != HAL_OK)
    {
      App_Log("epd not ready: %lu\r\n", (unsigned long)epd_status);
      return;
    }
    epd_status = EPD213_TestPattern(&g_epd);
    if (epd_status == HAL_OK)
    {
      App_Log("epd test ok\r\n");
    }
    else
    {
      App_Log("epd test fail: %lu\r\n", (unsigned long)epd_status);
    }
  }
  else if (strcmp(g_uart_cmd_buf, "epdsleep") == 0)
  {
    epd_status = EPD213_Sleep(&g_epd);
    if (epd_status == HAL_OK)
    {
      g_epd_inited = 0U;
      App_Log("epd sleep ok\r\n");
    }
    else
    {
      App_Log("epd sleep fail: %lu\r\n", (unsigned long)epd_status);
    }
  }
  else if (strcmp(g_uart_cmd_buf, "epdtext") == 0)
  {
    App_Log("usage: epdtext <msg>\r\n");
  }
  else if (strncmp(g_uart_cmd_buf, "epdtext ", 8U) == 0)
  {
    const char *msg = &g_uart_cmd_buf[8];

    epd_status = App_EpdEnsureInit();
    if (epd_status != HAL_OK)
    {
      App_Log("epd not ready: %lu\r\n", (unsigned long)epd_status);
      return;
    }
    if (msg[0] == '\0')
    {
      App_Log("usage: epdtext <msg>\r\n");
      return;
    }

    epd_status = EPD213_ShowText(&g_epd, msg, 4U, 8U, 1U);
    if (epd_status == HAL_OK)
    {
      App_Log("epd text ok\r\n");
    }
    else
    {
      App_Log("epd text fail: %lu\r\n", (unsigned long)epd_status);
    }
  }
  else if (strcmp(g_uart_cmd_buf, "epdscroll") == 0)
  {
    epd_status = App_EpdScrollText("AUTO SCROLL DEMO");
    if (epd_status == HAL_OK)
    {
      App_Log("epd scroll ok\r\n");
    }
    else
    {
      App_Log("epd scroll fail: %lu\r\n", (unsigned long)epd_status);
    }
  }
  else if (strncmp(g_uart_cmd_buf, "epdscroll ", 10U) == 0)
  {
    const char *msg = &g_uart_cmd_buf[10];

    if (msg[0] == '\0')
    {
      App_Log("usage: epdscroll [msg]\r\n");
      return;
    }

    epd_status = App_EpdScrollText(msg);
    if (epd_status == HAL_OK)
    {
      App_Log("epd scroll ok\r\n");
    }
    else
    {
      App_Log("epd scroll fail: %lu\r\n", (unsigned long)epd_status);
    }
  }
  else if (strcmp(g_uart_cmd_buf, "epdautoon") == 0)
  {
    g_epd_auto_refresh_enabled = 1U;
    g_epd_next_refresh_ms = g_tim6_ms - EPD_AUTO_REFRESH_INTERVAL_MS;
    App_Log("epd auto refresh=ON\r\n");
  }
  else if (strcmp(g_uart_cmd_buf, "epdautooff") == 0)
  {
    g_epd_auto_refresh_enabled = 0U;
    App_Log("epd auto refresh=OFF\r\n");
  }
  else
  {
    App_Log("unknown command: %s\r\n", g_uart_cmd_buf);
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  uint32_t now;

  if (GPIO_Pin != BTN_GPIO_PIN)
  {
    return;
  }

  now = HAL_GetTick();
  if ((now - g_last_btn_tick) < BTN_DEBOUNCE_MS)
  {
    return;
  }

  g_last_btn_tick = now;
  g_blink_enabled ^= 1U;
  g_btn_event_pending = 1U;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)
  {
    g_tim6_ms++;
  }
}

static HAL_StatusTypeDef App_EpdEnsureInit(void)
{
  HAL_StatusTypeDef status;

  if (g_epd_inited != 0U)
  {
    return HAL_OK;
  }

  status = EPD213_Init(&g_epd);
  if (status == HAL_OK)
  {
    g_epd_inited = 1U;
  }
  return status;
}

static HAL_StatusTypeDef App_EpdScrollText(const char *text)
{
  HAL_StatusTypeDef status;
  uint16_t y;
  uint16_t last_y;
  uint16_t final_y;

  if ((text == NULL) || (text[0] == '\0'))
  {
    return HAL_ERROR;
  }

  status = App_EpdEnsureInit();
  if (status != HAL_OK)
  {
    return status;
  }

  final_y = (uint16_t)(EPD213_HEIGHT - EPD_TEXT_HEIGHT_PX - 1U);
  last_y = 0U;
  for (y = 0U; y <= final_y; y = (uint16_t)(y + EPD_SCROLL_STEP_PX))
  {
    status = EPD213_ShowText(&g_epd, text, EPD_SCROLL_TEXT_X, y, 1U);
    if (status != HAL_OK)
    {
      return status;
    }
    last_y = y;
    HAL_Delay(EPD_SCROLL_DELAY_MS);
  }

  if (last_y != final_y)
  {
    status = EPD213_ShowText(&g_epd, text, EPD_SCROLL_TEXT_X, final_y, 1U);
    if (status != HAL_OK)
    {
      return status;
    }
  }

  return HAL_OK;
}

static void App_EpdAutoRefreshTask(void)
{
  HAL_StatusTypeDef status;
  uint32_t mode;
  char msg[32];

  if (g_epd_auto_refresh_enabled == 0U)
  {
    return;
  }
  if ((g_tim6_ms - g_epd_next_refresh_ms) < EPD_AUTO_REFRESH_INTERVAL_MS)
  {
    return;
  }

  status = App_EpdEnsureInit();
  if (status != HAL_OK)
  {
    App_Log("epd auto init fail: %lu\r\n", (unsigned long)status);
    g_epd_next_refresh_ms = g_tim6_ms;
    return;
  }

  mode = g_epd_refresh_count % 4U;
  if (mode == 0U)
  {
    status = EPD213_Clear(&g_epd);
  }
  else if (mode == 1U)
  {
    status = EPD213_TestPattern(&g_epd);
  }
  else if (mode == 2U)
  {
    (void)snprintf(msg, sizeof(msg), "PANEL TEST #%lu", (unsigned long)g_epd_refresh_count);
    status = EPD213_ShowText(&g_epd, msg, EPD_SCROLL_TEXT_X, 8U, 0U);
  }
  else
  {
    (void)snprintf(msg, sizeof(msg), "PANEL TEST #%lu", (unsigned long)g_epd_refresh_count);
    status = EPD213_ShowText(&g_epd, msg, EPD_SCROLL_TEXT_X, 8U, 1U);
  }

  if (status == HAL_OK)
  {
    App_Log("epd auto frame=%lu mode=%lu ok\r\n",
            (unsigned long)g_epd_refresh_count,
            (unsigned long)mode);
    g_epd_refresh_count++;
  }
  else
  {
    App_Log("epd auto frame=%lu mode=%lu fail: %lu\r\n",
            (unsigned long)g_epd_refresh_count,
            (unsigned long)mode,
            (unsigned long)status);
    g_epd_inited = 0U;
  }

  g_epd_next_refresh_ms = g_tim6_ms;
}

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
