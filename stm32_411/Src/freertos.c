/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "motorset.h"
#include "usart.h"
#include "ultrasonic.h"
#include "motorset.h"
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern volatile uint16_t adcValue[2];
extern volatile uint8_t distance_front, distance_left, distance_right;

#define ROTATION_GAIN 2.0f
#define MAX_DIFF      (DIST_MAX - DIST_MIN)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_JoystickRe */
osThreadId_t Task_JoystickReHandle;
const osThreadAttr_t Task_JoystickRe_attributes = {
  .name = "Task_JoystickRe",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Task_ManualDrv */
osThreadId_t Task_ManualDrvHandle;
const osThreadAttr_t Task_ManualDrv_attributes = {
  .name = "Task_ManualDrv",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Task_AutoDrv_Se */
osThreadId_t Task_AutoDrv_SeHandle;
const osThreadAttr_t Task_AutoDrv_Se_attributes = {
  .name = "Task_AutoDrv_Se",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for Task_AutoDrv_Co */
osThreadId_t Task_AutoDrv_CoHandle;
const osThreadAttr_t Task_AutoDrv_Co_attributes = {
  .name = "Task_AutoDrv_Co",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for qSensor */
osMessageQueueId_t qSensorHandle;
const osMessageQueueAttr_t qSensor_attributes = {
  .name = "qSensor"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask_JoystickRecv(void *argument);
void StartTask_ManualDrv(void *argument);
void StartTask_AutoDrv_Sensor(void *argument);
void StartTask_AutoDrv_Control(void *argument);

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
  /* creation of qSensor */
	qSensorHandle = osMessageQueueNew(3, sizeof(DistanceData_t), &qSensor_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of Task_JoystickRe */
  Task_JoystickReHandle = osThreadNew(StartTask_JoystickRecv, NULL, &Task_JoystickRe_attributes);

  /* creation of Task_ManualDrv */
  Task_ManualDrvHandle = osThreadNew(StartTask_ManualDrv, NULL, &Task_ManualDrv_attributes);

  /* creation of Task_AutoDrv_Se */
  Task_AutoDrv_SeHandle = osThreadNew(StartTask_AutoDrv_Sensor, NULL, &Task_AutoDrv_Se_attributes);

  /* creation of Task_AutoDrv_Co */
  Task_AutoDrv_CoHandle = osThreadNew(StartTask_AutoDrv_Control, NULL, &Task_AutoDrv_Co_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask_JoystickRecv */
/**
* @brief Function implementing the Task_JoystickRe thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask_JoystickRecv */
void StartTask_JoystickRecv(void *argument)
{
	uint8_t rx;
	char rxString[32];
	uint8_t rxIdx = 0;
	
	/* Infinite loop */
	for (;;)
	{
		// UART로부터 한 글자 수신 (non-blocking)
		if (HAL_UART_Receive(&huart2, &rx, 1, 10) == HAL_OK)
		{
			// 모드 전환 명령 체크 (단일 문자)
			if (rx == '1' && rxIdx == 0)  // 수동 모드
			{
				currentMode = MODE_MANUAL;
				motorStop();
				printf("Mode: MANUAL\r\n");
				continue;
			}
			else if (rx == '2' && rxIdx == 0)  // 자동 모드
			{
				currentMode = MODE_AUTO;
				motorStop();
				printf("Mode: AUTO\r\n");
				continue;
			}
			
			// 조이스틱 데이터 처리 (y,x 형식)
			if (rx == '\n' || rx == '\r')
			{
				if (rxIdx > 0)
				{
					rxString[rxIdx] = '\0';
					// 수동 모드일 때만 조이스틱 데이터 처리
					if (currentMode == MODE_MANUAL)
					{
						uint16_t y, x;
						if (sscanf(rxString, "%hu,%hu", &y, &x) == 2)
						{
							// 범위 체크
							if (y <= 4095) adcValue[0] = y;
							if (x <= 4095) adcValue[1] = x;
						}
					}
					rxIdx = 0;
				}
			}
			else if (rxIdx < 31)
			{
				rxString[rxIdx++] = rx;
			}
		}
		
		osDelay(20);  // 50Hz 체크 주기
	}
}

/* USER CODE BEGIN Header_StartTask_ManualDrv */
/**
* @brief Function implementing the Task_ManualDrv thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask_ManualDrv */
void StartTask_ManualDrv(void *argument)
{
  /* USER CODE BEGIN StartTask_ManualDrv */
	static DriveMode prevMode = MODE_MANUAL;
  /* Infinite loop */
  for(;;)
	{
		if (currentMode == MODE_MANUAL)
		{
			motorControlFromADC ();  // 조이스틱 기반 수동 제어
		}
		else
		{
			// 모드가 바뀐 순간이면 정지
			if (prevMode == MODE_MANUAL)
			{
				motorStop ();
			}
		}

		prevMode = currentMode;
		osDelay (20);  // 부드러운 조작 반응 주기
	}
  /* USER CODE END StartTask_ManualDrv */
}

/* USER CODE BEGIN Header_StartTask_AutoDrv_Sensor */
/**
* @brief Function implementing the Task_AutoDrv_Se thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask_AutoDrv_Sensor */
void StartTask_AutoDrv_Sensor(void *argument)
{
	DistanceData_t dist;
	uint8_t sensor_index = 0;
	
	/* Infinite loop */
	for(;;)
	{
		// 센서를 순차적으로 하나씩 측정 (더 빠른 업데이트)
		switch(sensor_index)
		{
			case 0:
				Trigger_Left();
				osDelay(30);  // 에코 대기 시간
				dist.left = distance_left;
				break;
			case 1:
				Trigger_Front();
				osDelay(30);
				dist.front = distance_front;
				break;
			case 2:
				Trigger_Right();
				osDelay(30);
				dist.right = distance_right;
				
				// 3개 센서 모두 측정 완료시 큐에 전송
				osMessageQueuePut(qSensorHandle, &dist, 0, 0);
				break;
		}
		
		sensor_index = (sensor_index + 1) % 3;
		
		// 총 주기: 90ms (3 센서 * 30ms)
		// 이전 180ms에서 90ms로 개선
	}
}

/* USER CODE BEGIN Header_StartTask_AutoDrv_Control */
/**
* @brief Function implementing the Task_AutoDrv_Co thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask_AutoDrv_Control */
void StartTask_AutoDrv_Control(void *argument)
{
	DistanceData_t dist;
	const float ROTATION_GAIN = 1.5f;  // 회전 게인 조정
	const uint8_t SAFETY_DISTANCE = DIST_MIN + 10;  // 안전거리 여유
	
	/* Infinite loop */
	for (;;)
	{
		if (currentMode != MODE_AUTO)
		{
			osDelay(50);
			continue;
		}

		if (osMessageQueueGet(qSensorHandle, &dist, NULL, 100) == osOK)  // 100ms 타임아웃
		{
			uint8_t front = dist.front;
			uint8_t left = dist.left;
			uint8_t right = dist.right;

			// 전방 장애물 회피 우선
			if (front <= SAFETY_DISTANCE)
			{
				// 긴급 정지 또는 후진
				if (front <= DIST_MIN)
				{
					adcValue[0] = ADC_BACK;  // 후진
				}
				else
				{
					adcValue[0] = ADC_Y_CENTER - 200;  // 느리게 후진
				}
				
				// 좌우 판단하여 회피 방향 결정
				if (left > right && left > SAFETY_DISTANCE)
				{
					adcValue[1] = 1000;  // 왼쪽으로 회피
				}
				else if (right > left && right > SAFETY_DISTANCE)
				{
					adcValue[1] = 3000;  // 오른쪽으로 회피
				}
			}
			else
			{
				// 전방 거리에 비례한 속도 설정
				uint16_t speed_factor = (front - DIST_MIN) * 1000 / (DIST_MAX - DIST_MIN);
				adcValue[0] = ADC_Y_CENTER + speed_factor;
				
				// 좌우 벽 추종
				int16_t diff = (int16_t)left - (int16_t)right;
				
				// 양쪽 벽이 너무 가까운 경우
				if (left <= SAFETY_DISTANCE && right <= SAFETY_DISTANCE)
				{
					adcValue[1] = ADC_X_CENTER;  // 직진
					adcValue[0] = ADC_Y_CENTER - 300;  // 속도 감소
				}
				// 왼쪽 벽만 가까운 경우
				else if (left <= SAFETY_DISTANCE)
				{
					adcValue[1] = ADC_X_CENTER + 800;  // 오른쪽으로
				}
				// 오른쪽 벽만 가까운 경우
				else if (right <= SAFETY_DISTANCE)
				{
					adcValue[1] = ADC_X_CENTER - 800;  // 왼쪽으로
				}
				// 중앙 유지
				else
				{
					// 차이가 크지 않으면 직진
					if (abs(diff) < 10)
					{
						adcValue[1] = ADC_X_CENTER;
					}
					else
					{
						// 부드러운 조향
						int16_t steering = (diff * 500) / (DIST_MAX - DIST_MIN);
						adcValue[1] = ADC_X_CENTER - steering;
					}
				}
			}
			
			// 범위 제한 (uint16_t이므로 음수 체크 불필요)
			if (adcValue[0] > 4095) adcValue[0] = 4095;
			if (adcValue[1] > 4095) adcValue[1] = 4095;
			
			// 디버그 출력 (옵션)
			// printf("F:%d L:%d R:%d -> Y:%d X:%d\r\n", 
			//        front, left, right, adcValue[0], adcValue[1]);
			
			motorControlFromADC();
		}
	}
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

