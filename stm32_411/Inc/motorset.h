#ifndef INC_MOTORSET_H_
#define INC_MOTORSET_H_

#include "main.h"
#include "tim.h"

// 모터 GPIO 정의
#define MOTOR_PORT GPIOB
#define MOTOR_In1  GPIO_PIN_13
#define MOTOR_In2  GPIO_PIN_14
#define MOTOR_In3  GPIO_PIN_15
#define MOTOR_In4  GPIO_PIN_1
#define on  1
#define off 0

// ADC 범위 정의 (12비트 ADC)
#define ADC_MAX 4095
#define ADC_MIN 0
#define ADC_BACK 800

// 조이스틱 중심값 및 데드존
#define ADC_Y_CENTER 2048  // 전후 중심
#define ADC_X_CENTER 2048  // 좌우 중심 (1950 -> 2048로 수정)
#define DEADZONE_Y   100   // 전후 데드존 (45 -> 100 증가)
#define DEADZONE_X   100   // 좌우 데드존 (45 -> 100 증가)

// PWM 설정
#define PWM_MAX      1000  // 최대 PWM (100% duty)
#define PWM_MIN      650   // 최소 PWM (750 -> 650 감소)
#define PWM_STEERING_MAX 1200  // 좌우바퀴 차이 한계값
#define PWM_ROTATE_MAX 700     // 제자리 회전 최대 속도
#define PWM_ROTATE_MIN 500     // 제자리 회전 최소 속도

// 자율주행 거리 설정
#define DIST_MAX 80     // 최대거리 (70 -> 80 증가)
#define DIST_MIN 20     // 최소거리 (25 -> 20 감소)
#define DIST_SAFE 35    // 안전거리 (새로 추가)

// 외부 변수
extern volatile uint16_t adcValue[2];

// 함수 프로토타입
void motorControlFromADC(void);
void motorForward(void);
void motorBackward(void);
void motorLeftTurn(void);
void motorRightTurn(void);
void motorStop(void);
void motorSetSpeed(int16_t leftSpeed, int16_t rightSpeed);  // 새로 추가

#endif /* INC_MOTORSET_H_ */