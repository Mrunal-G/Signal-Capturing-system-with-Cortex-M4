// Board Support Pack for Nucleo 476 Shield

/*
PA5, 6, 7, 8 are output push pull
PA1, 9, 10 are switch input with pull-up
Functions:
BSP476_Init()	Initialises I/O configuration for switches (0,1,2) and LEDs (0,1,2,3)
LED_On(LEDx)  Switches LEDx ON  (where x = 0,1,2,3)
LED_Off(LEDx) Switches LEDx OFF (where x = 0,1,2,3)
LED_Toggle(LEDx) Toggles LEDx   (where x = 0,1,2,3)
SWITCH_Read(SWx) Reads state of SWx(where x = 0,1,2)  
SW0 is push switch, SW1, SW2 are slide DIP switches
SW0 not pressed = ON.  SW1, SW2 'up' = ON
******************************************
Top of Board:
                  Switches    LEDs
   Reset   DAC    0    2  1   3210
  _________________________________
  ||___|    ..   | |  |4321|  **** |    
  |  _           | |  |    |       |
  | |0|Pot       |_|  |____|       |
  |  -                             |
  |  _                             |
  | |.|ADC                         |
  | |.|Link                        |
	|  -                             |
*/
#include "stm32L4xx.h"

#define LED0 5	//bit positions for LEDs
#define LED1 6
#define LED2 7
#define LED3 8
#define SW0  1	//bit positions for switches
#define SW1  9
#define SW2  10

void BSP476_Init()
{
	// Enable GPIOs clock  	
	RCC->AHB2ENR |=  RCC_AHB2ENR_GPIOAEN;
// Configure GPIOA5-8 as push-pull output high speed
	// GPIO Mode: Input(00), Output(01), AltFunc(10), Analog(11, reset)
	GPIOA->MODER &= ~(0xFFUL<<10);    // clear both bit 5-8 mode bits 
	GPIOA->MODER |= 0x55UL<<10;      	//  Output(01) set bit 5-8 for output
	
	// GPIO Speed: Low speed (00 = reset), Medium speed (01), Fast speed (10), High speed (11)
	GPIOA->OSPEEDR &= ~(0xFFUL<<(10));// reset value anyway
	GPIOA->OSPEEDR |=   0xFFUL<<(10); // High speed
	
	// GPIO Output Type: Output push-pull (0, reset), Output open drain (1) 
	GPIOA->OTYPER &= ~(0xFUL<<5);     // Push-pull is reset state
//Configure GPIOA 1, 9, 10 as digital input with pull-up	
	//GPIOA Mode Input(00), Output(01), AltFunc(10), Analog(11, reset)
	GPIOA->MODER &= ~(0x3C000CUL);
	// GPIO Input PUD (00 reset for b0-b12), Pull-up (01), Pull-down (10), Reserved (11)
	GPIOA->PUPDR  |= 0x140004UL;   		// pull-up 1, 9, 10	
}

void LED_On(uint16_t number)
{
	if (number==0) GPIOA->ODR |= 1UL<<LED0;		//switch ON
	if (number==1) GPIOA->ODR |= 1UL<<LED1;		//switch ON
	if (number==2) GPIOA->ODR |= 1UL<<LED2;		//switch ON
	if (number==3) GPIOA->ODR |= 1UL<<LED3;		//switch ON
}

void LED_Off(uint16_t number)
{
	if (number==0)	GPIOA->ODR &= ~(1UL<<LED0);	//switch OFF
	if (number==1)	GPIOA->ODR &= ~(1UL<<LED1);	//switch OFF
	if (number==2)	GPIOA->ODR &= ~(1UL<<LED2);	//switch OFF
	if (number==3)	GPIOA->ODR &= ~(1UL<<LED3);	//switch OFF
}

void LED_Toggle(uint16_t number)
{
	if (number==0)	GPIOA->ODR ^= 1UL<<LED0;	//Toggle
	if (number==1)	GPIOA->ODR ^= 1UL<<LED1;	//Toggle
	if (number==2)	GPIOA->ODR ^= 1UL<<LED2;	//Toggle
	if (number==3)	GPIOA->ODR ^= 1UL<<LED3;	//Toggle
}
uint16_t SWITCH_Read(uint16_t number)
{
	uint16_t state=0;
	if ((number==0) && (GPIOA->IDR & 1UL<<(SW0))) state = 1;
	if ((number==1) && (GPIOA->IDR & 1UL<<(SW1))) state = 1;
	if ((number==2) && (GPIOA->IDR & 1UL<<(SW2))) state = 1;
	return state;
}

