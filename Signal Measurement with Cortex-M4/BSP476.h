// header for BSP476.c
#include "stdint.h"										// needed to define uint8_t, etc

void BSP476_Init(void);
void LED_On(uint16_t number);					//LED number 0, 1, 2, 3 allowed
void LED_Off(uint16_t number);
void LED_Toggle(uint16_t number);
uint16_t SWITCH_Read(uint16_t number);	//Switch number 0, 1, 2 allowed

