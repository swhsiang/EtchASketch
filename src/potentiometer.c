#include "potentiometer.h"
#include "block.h"

uint32_t adc_helper_UPDN(uint32_t dr_check, uint32_t div){
	//This variable is static because we want to program to remember this value and alter it appropriately each time the function is called
	static uint32_t comp;
	uint32_t difference;
	//The idea behind this is to initialize the comp variable as 0 only before the data register of the adc has surpassed the threshold
	if(dr_check < div){
		comp = 0x0;
		
	}
	//Finding the difference between the comparison variable and the data register
	difference = comp - dr_check;
	//The next iteration (assuming the input has surpassed the div threshold) will compare its data register with its previous value and
	//return its difference
	comp = dr_check;
	return difference;
}

uint32_t adc_helper_RL(uint32_t dr_check, uint32_t div){
	//This variable is static because we want to program to remember this value and alter it appropriately each time the function is called
	static uint32_t comp;
	uint32_t difference;
	//The idea behind this is to initialize the comp variable as 0 only before the data register of the adc has surpassed the threshold
	if(dr_check < div){
		comp = 0x0;
		
	}
	//Finding the difference between the comparison variable and the data register
	difference = comp - dr_check;
	//The next iteration (assuming the input has surpassed the div threshold) will compare its data register with its previous value and
	//return its difference
	comp = dr_check;
	return difference;
	
}

void adc_init(void)	// for the potentiometers
{
	//int * block;
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;	// enable clock to Port C
	GPIOC->MODER |= 0xf;	// set PC0 and PC1 for analog input
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;	// enable clock to ADC unit
	RCC->CR2 |= RCC_CR2_HSI14ON;	// turn on hi-spd internal 14 MHz clock
	while(!(RCC->CR2 & RCC_CR2_HSI14RDY));	// wait for 14 MHz clock to be ready
	ADC1->CR |= ADC_CR_ADEN;	// enable ADC
	while(!(ADC1->ISR & ADC_ISR_ADRDY));	// wait for ADC to be ready
	while((ADC1->CR & ADC_CR_ADSTART));	// wait for ADCstart to be 0
	ADC1->CFGR1 |= ADC_CFGR1_CONT | ADC_CFGR1_SCANDIR; /* (2) */
	ADC1->SMPR |= ADC_SMPR1_SMPR_0 | ADC_SMPR1_SMPR_1 | ADC_SMPR1_SMPR_2; /* (4) */
}

void read_adc() {
	while(1)
	{
		ADC1->CHSELR = 0;	// unselect all ADC channels
		ADC1->CHSELR |=  ADC_CHSELR_CHSEL10;  //1<<10;	// select channel 10
		while(!(ADC1->ISR & ADC_ISR_ADRDY));	// wait for ADC ready
		ADC1->CR |= ADC_CR_ADSTART;	// start the ADC
		while(!(ADC1->ISR & ADC_ISR_EOC));	// wait for end of conversion
		//delay_cycles(5000000);

		//DAVIDS FUNCTION ACCEPTS A DISTANCE AND A DIRECTION
		//1. CHECK IF THE DATA REGISTER HAS CHANGED BY 2^n / (ROW_NUM OR COL_NUM) (THIS CAN BE IN THE POSITIVE OR NEGATIVE DIRECTIONS)
		//2. IF IT HAS, INITIATE THE SOFTWARE TRIGGER AND CALL THE DISPLAY FUNCTION
		//3. CALCULATE THE DISTANCE MOVED BY SUBTRACTING THE VALUE WITHIN THE DATA REGISTER BY THE PREVIOUS VALUE RECORDED
		//QUESTION: IS THIS VALUE ALWAYS GOING TO BE 2^n / (ROW_NUM OR COL_NUM) AND IS THIS STEP NECESSARY?
		//4. THE VALUE IN STEP 3 MUST BE THEN DIVIDED BY THE MINIMUM VALUE NEEDED TO UPDATE THE SCREEN 2^n / (ROW_NUM OR COL_NUM)
		//PERHAPS THE DISTANCE SENT TO DAVIDS FUNCITON IS ALWAYS GOING TO BE 1.
		//IT IS STILL NECESSARY TO FIND THE DIFFERENCE BETWEEN THE DATA REGISTER AND ITS PREVIOUS VALUE
		uint32_t bit_res = 4095;
		uint32_t div = (bit_res / ROW_NUM);
		//This function will hopefully find the difference between the data register and its previous value
		uint32_t difference = adc_helper_UPDN(ADC1->DR , div);
		int direction;
		//Now we must find the direction
		if(difference > div){
			direction = 1;
			//NOW WE MUST SEND THE ARGUMENTS TO DAVIDS FUNCTION
			//davids_function(1,direction)
		}
		else if(difference < (-1*div)){
			direction = 0;
			//NOW WE MUST SEND THE ARGUMENTS TO DAVIDS FUNCTION
			//davids_function(1,direction)
		}

		

		ADC1->CHSELR = 0;	// unselect all ADC channels
		ADC1->CHSELR |=  ADC_CHSELR_CHSEL11; //1<<11;	// select channel 11
		while(!(ADC1->ISR & ADC_ISR_ADRDY));	// wait for ADC ready
		ADC1->CR |= ADC_CR_ADSTART;	// start the ADC
		while(!(ADC1->ISR & ADC_ISR_EOC));	// wait for end of conversion
		uint32_t div2 = bit_res / COLUMN_NUM;
		//This function will hopefully find the difference between the data register and its previous value
		uint32_t difference2 = adc_helper_RL(ADC1->DR , div2);
		//Now we must find the direction
		if(difference2 > div2){
			direction = 2;
			//NOW WE MUST SEND THE ARGUMENTS TO DAVIDS FUNCTION
			//davids_function(1,direction)
		}
		else if(difference2 < (-1*div2)){
			direction = 3;
			//NOW WE MUST SEND THE ARGUMENTS TO DAVIDS FUNCTION
			//davids_function(1,direction)
		}
	}
}
