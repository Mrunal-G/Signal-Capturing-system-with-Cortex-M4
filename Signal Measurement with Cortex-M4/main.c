#include "stm32l4xx.h"                  /* Device header */
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_usart.h"
#include "stdio.h"
#include "string.h"
#include "BSP476.h"
#include "math.h"

#define USARTx USART2					//Use USART2
#define USART_Tx LL_GPIO_PIN_2  				//Tx on PA2 


//prototype
void send_char(uint8_t);
void MinMax(float samples[],int size,float *min,float *max);
void RCC_init(void);
void GPIO_init(void);
void ADC12_init (void);
void DAC_init(void);
void SysTick_init(void);
void SysTick_Handler(void);

/* global variables */

// for terminal 
char text[300];


float samples[1000]; 

// average variables
float total = 0;
float  avg;

// min and max variables
float min,max;

// RMS variables
float RMS;
float sum=0;
float sqsum;
float result;

// Frequency variables
int z;
float count = 0;
float frequency;



int main(void){
	
	LL_GPIO_InitTypeDef  GPIO_InitStruct;			// declare structure for GPIO
  LL_USART_InitTypeDef USART_InitStruct;			// declare structure for USART
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);	// Enable GPIOA clock
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2); 	// Enable USART2 clock
	
	// right click "LL_APB1_GRP1_PERIPH_USART2" from above line and go to definition of it to search for the clock required for USART2, PA2
	
  GPIO_InitStruct.Pin = USART_Tx;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;		//Alternate function
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);			//configure PA2 as Tx output
	
  USART_InitStruct.BaudRate = 4800;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USARTx, &USART_InitStruct);
  LL_USART_Enable(USARTx);					//Enable USART 
	

		
    printf("application start.\n");
		printf("press user button to start new acquisition\n");
		
	RCC_init(); 		/* configure RCC */
	GPIO_init(); 		/* configure GPIO */
	ADC12_init(); 		/* initialize ADC12 */
	DAC_init(); 		/* initialize DAC*/
	SysTick_init(); 		/* configure SysTick timer */
	BSP476_Init();	



		while(1){
				
			if(!SWITCH_Read(0)){
						
				 int x ;
		
		for(x=0; x<1000; x++){

			    samples[x] = ADC1->DR;
			    total = total + samples[x];//loop for calculatin total
			    sum = sum + pow(samples[x],2);
			    //printf(" %f\n", samples[x]);	// put break point here and Run(f5) in debug to check for 3.1 volt led test (set oscilloscope for 3.3Vpp and 1.65V)
			    if(samples[x] >= 3812){
					 LED_On(1);
				 }
				
				 
		} // end of for loop 
		
	  	    
		// for calculating the min and max voltages
		MinMax(samples,1000,&min,&max); // size of array "samples" is defined as 1000 here which is passed to the function
		
		
		// Average voltage
		avg = (total/1000) ;//calculate average
		float Volt_avg = (3.33f*(avg/4096));
    printf("The average is:%2.2f \n",avg);
		printf("The average in volts is:%2.2f \n",Volt_avg);
		total = 0; // to reset the total after printing the average one time
		
		 // RMS calculation
     sqsum = (sum/1000);
     result=sqrt(sqsum);
		 RMS = (3.33f*(result/4096));
		 printf("The RMS voltage = %2.2f \n",RMS);
		 sum = 0; // to reset the total after printing the average one time

		// minimum  calculation
		printf("Min value is %2.2f\n",min);
		float Volt_min = (3.33f*(min/4096));
		printf("Minimum Voltage = %2.2f\n",Volt_min);
		
		// maximum calculation
		printf("Max value is %2.2f\n",max);
		float Volt_max = (3.33f*(max/4096));
		printf("Maximum Volatge = %2.2f\n",Volt_max);	
		
		
		
		for(z=0; z<1000; z++){ 
			
			if((3.33f*(samples[z+1]/4096)) < RMS && (3.33f*(samples[z-1]/4096)) > RMS){
				count++;
			 }
			  frequency = (count*5);
		}
		count = 0;
   	printf(" freq : %2.2f\n",frequency); 
						
		sprintf(text,"\n ==========================\nAverage voltage: %2.2f\nMinimum voltage : %2.2f\nMaximum voltage   : %2.2f\nRMS voltage : %2.2f\nFrequency: %2.2f\n",Volt_avg, Volt_min, Volt_max, RMS, frequency);
		
		int length = strlen(text);
		for(int k=0; k<(length-1); k++){
			send_char(text[k]);
			for (uint32_t i=0; i<500; i++){}
		}
				
			} 
			
			DAC->DHR12R2 	= (uint32_t)(ADC1->DR*0.8);
				
			} // end of while(1)
		
		
} //end of main()


		
// Calulating min and max function 
void MinMax(float samples[],int size,float *min,float *max)
{
	if(size==0)
	return;
	else
	{
	*min = samples[0];
	*max = samples[0];
	int i;
	for(i=0;i<size;i++)
	{
			if(samples[i]>*max)
			*max = samples[i];
  
			if(samples[i]<*min)
			*min = samples[i];
	}
	}
}




void RCC_init(void){	
	RCC->AHB2ENR  |=  RCC_AHB2ENR_GPIOAEN;          	/* clock for GPIOA */ 	
	RCC->APB1ENR1 |=  RCC_APB1ENR1_DAC1EN; 		/* clock for DAC */
	RCC->CCIPR 	&= ~(3UL<<RCC_CCIPR_ADCSEL_Pos); 	/* reset clock selection of ADC */
	RCC->CCIPR         |=   3UL<<RCC_CCIPR_ADCSEL_Pos; 	/* 11 - system clock selected, i.e., 4MHz */
	RCC->AHB2ENR  |=  RCC_AHB2ENR_ADCEN;           	/* clock for ADC1  	*/
}



uint8_t y = 0x4D; // data to be transmitted 0x4D is the ASCII for 'M'

void GPIO_init(void){
	/* PA0 - input - analog - for ADC1_IN5*/
	uint32_t tmp 	= 0;
	GPIOA->MODER   &= ~(3UL<<(2*tmp));			/* 00 - input */
	GPIOA->MODER   |=   3UL<<(2*tmp);			/* 01 - general purpose output */
	GPIOA->OTYPER  &= ~(1UL<<tmp);			/* 0 - push-pull */
	GPIOA->OSPEEDR &= ~(3UL<<(2*tmp)); 		/* reset value anyway */
	GPIOA->OSPEEDR |=   0UL<<(2*tmp); 			/* 00 - low speed */	
	GPIOA->ASCR    |=   1UL<<tmp; 		/* 1  - analog switch control for ADC input (#####)*/
	/* PA5 - output - push-pull - for DAC_OUT2*/
	tmp 		= 5;
	GPIOA->MODER   &= ~(0x3UL<<(2*tmp)); 		/* 00-input */
	GPIOA->MODER   |= 0x1UL<<(2*tmp);			/* 01 - general purpose output*/
	GPIOA->OTYPER  &= ~(1UL<<tmp);			/* 0 - push-pull */
	GPIOA->OSPEEDR &= ~(0x3UL<<(2*tmp)); 		/* reset value anyway */
	GPIOA->OSPEEDR |=  0x3UL<<(2*tmp); 		/* 11 - very high speed */
	
	
}



void ADC12_init (void) {	
	/*sampling time*/	
	ADC1->SMPR1 |= 0ul<<ADC_SMPR1_SMP5_Pos; 		/* 000 - 2.5 ADC clock cycles */
	/* conversion mode: continuous */
	ADC1->CFGR |=  1ul<<ADC_CFGR_CONT_Pos; 		/* 1 - continous conversion */
	/* select ADC12_IN5 as 1st conversion in regular sequence */
	ADC1->SQR1 &=  ~( 0xFul<<ADC_SQR1_L_Pos );		/* 0000 - 1 conversion*/
	ADC1->SQR1 |=       0ul<<ADC_SQR1_L_Pos; 		/* 0000 - 1 conversion */	
	ADC1->SQR1 &= ~( 0x1Ful<<ADC_SQR1_SQ1_Pos );  	/*00000 - clear bitfield SQ1[4:0]*/
	ADC1->SQR1 |=       5ul<<ADC_SQR1_SQ1_Pos; 		/* 5 - select ADC12_IN5 as 1st conversion */
	ADC1->CR   &=  ~(1ul<<ADC_CR_DEEPPWD_Pos);     /* exit deep-power down, [RM0351,v2018,p511] */
	ADC1->CR   |=    1ul<<ADC_CR_ADVREGEN_Pos; 	 /* enable voltage regulator, [RM0351,v2018,p511] */
	ADC1->CR   |= 1ul<<ADC_CR_ADCAL_Pos; 		/*ADC calibration starts*/
	while( ADC1->CR & ADC_CR_ADCAL ){}			/* wait until sel-calibration completes */
	ADC1->CR   |=  ADC_CR_ADEN;			/* ADC1 enable*/
	while( !(ADC1->ISR & ADC_ISR_ADRDY) ){} 		/* wait until ADC is ready for conversion */
	ADC1->CR     |=  1ul<<ADC_CR_ADSTART_Pos; 		/* ADC start conversion */
}


void DAC_init(void){		 
	DAC->DHR12R2 	 = 0;				/* DAC_output2 = 0v */
	DAC->CR 		|= 1UL<<DAC_CR_EN2_Pos; 		/* enable DAC_OUT2 */	
}

void SysTick_init(void){
	SysTick->LOAD  = 4000 - 1; 			/* set period in system clock cycles */
	SysTick->VAL   = 0; 				/* initialize counter within SysTick timer */
	SysTick->CTRL |= 7UL<<0; 			/* start running & listening to SysTick interrupts */
}


void SysTick_Handler(void){
	ADC1->ISR    |=  1UL<<ADC_ISR_EOC_Pos; 	/*clear EOC flag*/     
}

void send_char(uint8_t ch)
{  
  while (!LL_USART_IsActiveFlag_TXE(USARTx)); 	// Wait for TXE flag to be raised  // to transmit data to serial port
  LL_USART_TransmitData8(USARTx, ch); 

}


