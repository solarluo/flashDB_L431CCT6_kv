/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "st7735.h"
#include "lcd_text.h"
#include "aht30.h"
#include "rtc.h"
#include "flashdb.h"
#include "usart.h"
#include "fal.h"
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
/* USER CODE BEGIN Variables */
static struct fdb_tsdb tsdb;
static uint8_t tsdb_inited = 0;

typedef struct
{
    int temp;
    int humi;
} sensor_log_t;

/* RTC -> 时间戳（简化版，保证单调递增即可） */
static fdb_time_t tsdb_get_time(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    return (fdb_time_t)(sTime.Hours * 3600 + sTime.Minutes * 60 + sTime.Seconds);
}

static fdb_err_t MX_TSDB_Init(void)
{
    return fdb_tsdb_init(&tsdb, "tsdb1", "fdb_ts", tsdb_get_time, sizeof(sensor_log_t), NULL);
}

static void TSDB_Append_One(int temp, int humi)
{
    sensor_log_t log_data;
    struct fdb_blob blob;

    log_data.temp = temp;
    log_data.humi = humi;

    fdb_tsl_append(&tsdb, fdb_blob_make(&blob, &log_data, sizeof(log_data)));
}

static bool tsdb_query_cb(fdb_tsl_t tsl, void *arg)
{
    sensor_log_t log_data;
    struct fdb_blob blob;
    char msg[128];

    fdb_blob_read((fdb_db_t)&tsdb,
                  fdb_tsl_to_blob(tsl, fdb_blob_make(&blob, &log_data, sizeof(log_data))));

    snprintf(msg, sizeof(msg),
             "TSDB: ts=%ld temp=%d humi=%d\r\n",
             (long)tsl->time,
             log_data.temp,
             log_data.humi);

    HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

    return false;
}

static void TSDB_Read_All(void)
{
    fdb_tsl_iter(&tsdb, tsdb_query_cb, NULL);
}
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myTask02 */
osThreadId_t myTask02Handle;
const osThreadAttr_t myTask02_attributes = {
  .name = "myTask02",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for myTask03 */
osThreadId_t myTask03Handle;
const osThreadAttr_t myTask03_attributes = {
  .name = "myTask03",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of myTask02 */
  myTask02Handle = osThreadNew(StartTask02, NULL, &myTask02_attributes);

  /* creation of myTask03 */
  myTask03Handle = osThreadNew(StartTask03, NULL, &myTask03_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
* @brief Function implementing the defaultTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  char buf[32];
  char msg[128];
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  uint32_t sample_cnt = 0;
  uint8_t last_log_sec = 255;
  fdb_err_t ts_ret;

  
  LCD_ClearArea(0, 0, 80, 80, ST7735_BLACK);

  ts_ret = MX_TSDB_Init();
  snprintf(msg, sizeof(msg), "fdb_tsdb_init = %d\r\n", ts_ret);
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

  if (ts_ret == FDB_NO_ERR)
  {
      tsdb_inited = 1;
  }

  for(;;)
  {
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    snprintf(buf, sizeof(buf),
             "20%02d-%02d-%02d %02d:%02d:%02d\r\n",
             sDate.Year, sDate.Month, sDate.Date,
             sTime.Hours, sTime.Minutes, sTime.Seconds);

    LCD_ShowString(0, 0, buf, ST7735_YELLOW, ST7735_BLACK);

    if (tsdb_inited && (sTime.Seconds % 5 == 0) && (sTime.Seconds != last_log_sec))
    {
        int temp_demo = 250 + sample_cnt;
        int humi_demo = 600 + sample_cnt;
        fdb_err_t append_ret;
        sensor_log_t log_data;
        struct fdb_blob blob;

        log_data.temp = temp_demo;
        log_data.humi = humi_demo;

        append_ret = fdb_tsl_append(&tsdb, fdb_blob_make(&blob, &log_data, sizeof(log_data)));

        snprintf(msg, sizeof(msg),
                 "append ret=%d temp=%d humi=%d sec=%d\r\n",
                 append_ret, temp_demo, humi_demo, sTime.Seconds);
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

        sample_cnt++;
        last_log_sec = sTime.Seconds;

        if (sample_cnt == 3)
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"read all tsdb\r\n",
                              strlen("read all tsdb\r\n"), HAL_MAX_DELAY);
            TSDB_Read_All();
        }
    }

    if (sTime.Seconds % 5 != 0)
    {
        last_log_sec = 255;
    }

    osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  uint8_t cc = 1; 
  /* Infinite loop */
  for(;;)
  {
    //LCD_ShowUInt(0, 80, cc, ST7735_GREEN, ST7735_WHITE);
    osDelay(100);
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_9);
    cc++;
    
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the myTask03 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTask03 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

