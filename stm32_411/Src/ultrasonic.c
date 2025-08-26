#include "ultrasonic.h"

volatile uint8_t distance_front = 0;
volatile uint8_t distance_left  = 0;
volatile uint8_t distance_right = 0;

volatile uint16_t IC1_1 = 0, IC1_2 = 0;
volatile uint16_t IC2_1 = 0, IC2_2 = 0;
volatile uint16_t IC3_1 = 0, IC3_2 = 0;

volatile uint8_t flag1 = 0, flag2 = 0, flag3 = 0;

void Trigger_Left(void)
{
	HAL_GPIO_WritePin (GPIOA, GPIO_PIN_5, 0);
	delay_us (2);
	HAL_GPIO_WritePin (GPIOA, GPIO_PIN_5, 1);
	delay_us (10);
	HAL_GPIO_WritePin (GPIOA, GPIO_PIN_5, 0);

	__HAL_TIM_ENABLE_IT(&htim4, TIM_IT_CC1); // Channel 1 = Left
}

void Trigger_Front(void)
{
	HAL_GPIO_WritePin (GPIOA, GPIO_PIN_6, 0);
	delay_us (2);
	HAL_GPIO_WritePin (GPIOA, GPIO_PIN_6, 1);
	delay_us (10);
	HAL_GPIO_WritePin (GPIOA, GPIO_PIN_6, 0);
	__HAL_TIM_ENABLE_IT(&htim4, TIM_IT_CC3); // Channel 3 = Front
}

void Trigger_Right (void)
{
	HAL_GPIO_WritePin (GPIOA, GPIO_PIN_7, 0);
	delay_us (2);
	HAL_GPIO_WritePin (GPIOA, GPIO_PIN_7, 1);
	delay_us (10);
	HAL_GPIO_WritePin (GPIOA, GPIO_PIN_7, 0);
	__HAL_TIM_ENABLE_IT(&htim4, TIM_IT_CC2); // Channel 2 = Right
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	// Channel 1: Left Sensor
	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
	{
		if(flag1 == 0)
		{
			IC1_1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			flag1 = 1;
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
		}
		else
		{
			IC1_2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			flag1 = 0;
			uint16_t echoTime = (IC1_2 > IC1_1) ? (IC1_2 - IC1_1) : (0xFFFF - IC1_1 + IC1_2);
			distance_left = echoTime / 58;  // 수정: left는 left에 저장
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
			__HAL_TIM_DISABLE_IT(htim, TIM_IT_CC1);
		}
	}

	// Channel 2: Right Sensor
	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
	{
		if(flag2 == 0)
		{
			IC2_1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
			flag2 = 1;
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_FALLING);
		}
		else
		{
			IC2_2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
			flag2 = 0;
			uint16_t echoTime = (IC2_2 > IC2_1) ? (IC2_2 - IC2_1) : (0xFFFF - IC2_1 + IC2_2);
			distance_right = echoTime / 58;  // 수정: right는 right에 저장
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
			__HAL_TIM_DISABLE_IT(htim, TIM_IT_CC2);
		}
	}

	// Channel 3: Front Sensor
	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
	{
		if(flag3 == 0)
		{
			IC3_1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
			flag3 = 1;
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_FALLING);
		}
		else
		{
			IC3_2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
			flag3 = 0;
			uint16_t echoTime = (IC3_2 > IC3_1) ? (IC3_2 - IC3_1) : (0xFFFF - IC3_1 + IC3_2);
			distance_front = echoTime / 58;  // 수정: front는 front에 저장
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);
			__HAL_TIM_DISABLE_IT(htim, TIM_IT_CC3);
		}
	}
}