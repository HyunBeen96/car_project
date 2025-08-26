
#ifndef INC_ULTRASONIC_H_
#define INC_ULTRASONIC_H_

#include "main.h"
#include "tim.h"
#include "delay.h"

typedef struct {
  uint8_t front;
  uint8_t left;
  uint8_t right;
} DistanceData_t;

extern volatile uint8_t distance_front;
extern volatile uint8_t distance_left;
extern volatile uint8_t distance_right;

void Trigger_Left(void);
void Trigger_Front(void);
void Trigger_Right(void);




#endif /* INC_ULTRASONIC_H_ */
