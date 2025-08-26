#ifndef INC_RASPI_H_
#define INC_RASPI_H_

#define RX_BUFFER_SIZE 32
#include <stdint.h>

extern volatile uint8_t rxBuffer[RX_BUFFER_SIZE];  // 선언
extern volatile uint8_t stringReady;
extern volatile uint8_t rxIndex;
extern volatile uint8_t rxData;


#endif /* INC_RASPI_H_ */
