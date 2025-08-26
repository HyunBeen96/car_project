#include "motorset.h"
#include <stdlib.h>

// 새로운 함수: 모터 속도 직접 설정
void motorSetSpeed(int16_t leftSpeed, int16_t rightSpeed)
{
    // 방향 설정
    if (leftSpeed >= 0) {
        HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In1, on);
        HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In2, off);
    } else {
        HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In1, off);
        HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In2, on);
        leftSpeed = -leftSpeed;
    }
    
    if (rightSpeed >= 0) {
        HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In3, on);
        HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In4, off);
    } else {
        HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In3, off);
        HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In4, on);
        rightSpeed = -rightSpeed;
    }
    
    // PWM 설정
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, rightSpeed);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, leftSpeed);
}

void motorControlFromADC()
{
    int16_t forward_back = ADC_Y_CENTER - adcValue[0];  // +면 전진, -면 후진
    int16_t left_right = ADC_X_CENTER - adcValue[1];    // +면 왼쪽, -면 오른쪽
    
    int16_t leftSpeed, rightSpeed;
    
    // Deadzone 처리
    if (abs(forward_back) < DEADZONE_Y) forward_back = 0;
    if (abs(left_right) < DEADZONE_X) left_right = 0;
    
    // 정지 상태 체크
    if (forward_back == 0 && left_right == 0) {
        motorStop();
        return;
    }
    
    // 제자리 회전 (전후 입력 없이 좌우만 있는 경우)
    if (forward_back == 0 && left_right != 0) {
        float rotate_scale = (float)abs(left_right) / ADC_X_CENTER;
        float curved = rotate_scale * rotate_scale;  // 곡선 스케일링
        int16_t rotateSpeed = (int16_t)(curved * (PWM_ROTATE_MAX - PWM_ROTATE_MIN)) + PWM_ROTATE_MIN;
        
        if (left_right > 0) {  // 좌회전
            HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In1, on);
            HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In2, off);
            HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In3, off);
            HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In4, on);
        } else {  // 우회전
            HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In1, off);
            HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In2, on);
            HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In3, on);
            HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In4, off);
        }
        
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, rotateSpeed);
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, rotateSpeed);
        return;
    }
    
    // 전진/후진 속도 계산 (곡선 스케일링 적용)
    float scale = (float)abs(forward_back) / ADC_Y_CENTER;
    float curved = scale * scale;  // 제곱으로 부드러운 가속
    int16_t baseSpeed = (int16_t)(curved * (PWM_MAX - PWM_MIN)) + PWM_MIN;
    
    // 조향 계산 (차동 구동)
    int16_t steeringOffset = 0;
    if (left_right != 0) {
        // 속도에 비례한 조향 (빠를수록 조향 감소)
        float steeringFactor = 1.0f - (scale * 0.3f);  // 최대 속도에서 70% 조향
        steeringOffset = (int16_t)((left_right * PWM_STEERING_MAX * steeringFactor) / ADC_X_CENTER);
    }
    
    // 좌우 모터 속도 계산
    leftSpeed = baseSpeed - steeringOffset;
    rightSpeed = baseSpeed + steeringOffset;
    
    // 속도 제한
    if (leftSpeed > PWM_MAX) leftSpeed = PWM_MAX;
    if (leftSpeed < 0) leftSpeed = 0;
    if (rightSpeed > PWM_MAX) rightSpeed = PWM_MAX;
    if (rightSpeed < 0) rightSpeed = 0;
    
    // 방향 설정
    if (forward_back > 0) {  // 전진
        HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In1, on);
        HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In2, off);
        HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In3, on);
        HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In4, off);
    } else {  // 후진
        HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In1, off);
        HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In2, on);
        HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In3, off);
        HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In4, on);
    }
    
    // PWM 설정
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, rightSpeed);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, leftSpeed);
}

void motorForward()
{
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In1, on);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In2, off);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In3, on);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In4, off);
    
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 700);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 700);
}

void motorBackward()
{
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In1, off);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In2, on);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In3, off);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In4, on);
    
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 700);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 700);
}

void motorLeftTurn()
{
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In1, on);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In2, off);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In3, off);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In4, on);
    
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 600);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 600);
}

void motorRightTurn()
{
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In1, off);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In2, on);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In3, on);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In4, off);
    
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 600);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 600);
}

void motorStop()
{
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In1, off);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In2, off);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In3, off);
    HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_In4, off);
    
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
}