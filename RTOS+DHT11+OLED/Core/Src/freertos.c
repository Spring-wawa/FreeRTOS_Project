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
#include "OLED.h"
#include "dht11.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "fan.h"

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

osMessageQueueId_t queueDataHandle;   // ¸ø OLED
osMessageQueueId_t queueReportHandle; 

uint8_t temp = 0;
uint8_t humi = 0;
/* USER CODE END Variables */
/* Definitions for TaskLED */
osThreadId_t TaskLEDHandle;
const osThreadAttr_t TaskLED_attributes = {
  .name = "TaskLED",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskDHT11 */
osThreadId_t TaskDHT11Handle;
const osThreadAttr_t TaskDHT11_attributes = {
  .name = "TaskDHT11",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskOLED */
osThreadId_t TaskOLEDHandle;
const osThreadAttr_t TaskOLED_attributes = {
  .name = "TaskOLED",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskReport */
osThreadId_t TaskReportHandle;
const osThreadAttr_t TaskReport_attributes = {
  .name = "TaskReport",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for TaskFAN */
osThreadId_t TaskFANHandle;
const osThreadAttr_t TaskFAN_attributes = {
  .name = "TaskFAN",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myQueue */
osMessageQueueId_t myQueueHandle;
const osMessageQueueAttr_t myQueue_attributes = {
  .name = "myQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartTaskLED(void *argument);
void StartTaskDHT11(void *argument);
void StartTaskOLED(void *argument);
void StartTaskReport(void *argument);
void StartTaskFAN(void *argument);

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

  /* Create the queue(s) */
  /* creation of myQueue */
  myQueueHandle = osMessageQueueNew (2, sizeof(uint16_t), &myQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  queueDataHandle = osMessageQueueNew(2, sizeof(DHT11_Data_t), NULL); 
  queueReportHandle = osMessageQueueNew(2, sizeof(DHT11_Data_t), NULL);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of TaskLED */
  TaskLEDHandle = osThreadNew(StartTaskLED, NULL, &TaskLED_attributes);

  /* creation of TaskDHT11 */
  TaskDHT11Handle = osThreadNew(StartTaskDHT11, NULL, &TaskDHT11_attributes);

  /* creation of TaskOLED */
  TaskOLEDHandle = osThreadNew(StartTaskOLED, NULL, &TaskOLED_attributes);

  /* creation of TaskReport */
  TaskReportHandle = osThreadNew(StartTaskReport, NULL, &TaskReport_attributes);

  /* creation of TaskFAN */
  TaskFANHandle = osThreadNew(StartTaskFAN, NULL, &TaskFAN_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartTaskLED */
/**
  * @brief  Function implementing the TaskLED thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTaskLED */
void StartTaskLED(void *argument)
{
  /* USER CODE BEGIN StartTaskLED */

	
  /* Infinite loop */
  for(;;)
  {
	  HAL_GPIO_TogglePin(GPIOA,LED_Pin);
	  
    osDelay(500);
  }
  /* USER CODE END StartTaskLED */
}

/* USER CODE BEGIN Header_StartTaskDHT11 */
/**
* @brief Function implementing the TaskDHT11 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskDHT11 */
void StartTaskDHT11(void *argument)
{
  /* USER CODE BEGIN StartTaskDHT11 */
	DHT11_Data_t sensorData;
    char dbg[50];
    uint8_t ret;
	
  /* Infinite loop */
  for(;;)
  {
	  ret = DHT11_Read(&sensorData);
        if ( ret)
        {
            sprintf(dbg, "DHT11 OK: %dC, %d%%\r\n", sensorData.temperature, sensorData.humidity);
            osMessageQueuePut(queueDataHandle, &sensorData, 0, 0);
            osMessageQueuePut(queueReportHandle, &sensorData, 0, 0);
        }
        else
        {	
            sprintf(dbg, "DHT11 Error: %d\r\n", ret);
        }
        HAL_UART_Transmit(&huart1, (uint8_t*)dbg, strlen(dbg), 100);
        osDelay(2000);
  }
  /* USER CODE END StartTaskDHT11 */
}

/* USER CODE BEGIN Header_StartTaskOLED */
/**
* @brief Function implementing the TaskOLED thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskOLED */
void StartTaskOLED(void *argument)
{
  /* USER CODE BEGIN StartTaskOLED */
	 DHT11_Data_t data;
	
    
    char line[20];
  /* Infinite loop */
  for(;;)
  {
	    if (osMessageQueueGet(queueDataHandle, &data, NULL, osWaitForever) == osOK)
        {
            OLED_Clear();
            sprintf(line, "Temp: %02d C", data.temperature);
            OLED_ShowString(1, 1, line);
            sprintf(line, "Humi: %02d %%", data.humidity);
            OLED_ShowString(2, 1, line);
        }
    
  }
  /* USER CODE END StartTaskOLED */
}

/* USER CODE BEGIN Header_StartTaskReport */
/**
* @brief Function implementing the TaskReport thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskReport */
void StartTaskReport(void *argument)
{
  /* USER CODE BEGIN StartTaskReport */
	 DHT11_Data_t data;
    char json[60];
  /* Infinite loop */
  for(;;)
  {
	   if (osMessageQueueGet(queueDataHandle, &data, NULL, osWaitForever) == osOK)
        {
            sprintf(json, "{\"temperature\":%d,\"humidity\":%d}\r\n", data.temperature, data.humidity);
            HAL_UART_Transmit(&huart1, (uint8_t*)json, strlen(json), 100);
        }
    
  }
  /* USER CODE END StartTaskReport */
}

/* USER CODE BEGIN Header_StartTaskFAN */
/**
* @brief Function implementing the TaskFAN thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskFAN */
void StartTaskFAN(void *argument)
{
  /* USER CODE BEGIN StartTaskFAN */
  /* Infinite loop */
  DHT11_Data_t sensorData;
    uint8_t level = 0; 

    for(;;)
    {
 if(osMessageQueueGet(queueReportHandle, &sensorData, NULL, osWaitForever) == osOK)
        {
            uint8_t t = sensorData.temperature;

            switch(level)
            {
                case 0:
                    if(t >= 29) level = 1;
                    break;

                case 1:
                    if(t >= 32) level = 2;
                    else if(t <= 27) level = 0;
                    break;

                case 2:
                    if(t >= 35) level = 3;
                    else if(t <= 30) level = 1;
                    break;

                case 3:
                    if(t <= 33) level = 2;
                    break;
            }

            switch(level)
            {
                case 0:
                    Fan_Stop();
                    break;

                case 1:
                    Fan_SetSpeed(500);
                    break;

                case 2:
                    Fan_SetSpeed(600);
                    break;

                case 3:
                    Fan_SetSpeed(700);
                    break;
            }
        }

        osDelay(2000);
    }
    
  /* USER CODE END StartTaskFAN */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

