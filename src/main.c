/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

void display_gpio_init(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;	// enable GPIOB
	// init PB0 ~ PB9
	GPIOB->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2
			| GPIO_MODER_MODER3 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6
			| GPIO_MODER_MODER7 | GPIO_MODER_MODER8 | GPIO_MODER_MODER9);

	// set PB0 ~ PB9 to output mode
	GPIOB->MODER |= GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2_0
			| GPIO_MODER_MODER3_0 | GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0
			| GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0
			| GPIO_MODER_MODER9_0;
}

// ADC
void adc_init(void) {
	// enable clock to ADC unit
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	// turn on hi-spd internal 14 MHz clock
	RCC->CR2 |= RCC_CR2_HSI14ON;
	// wait for 14 MHz clock to be ready
	while(!(RCC->CR2 & RCC_CR2_HSI14RDY));
	// enable ADC
	ADC1->CR |= ADC_CR_ADEN;
	// wait for ADC to be ready
	while(!(ADC1->ISR & ADC_ISR_ADRDY));
	// wait for ADCstart to be 0
	while((ADC1->CR & ADC_CR_ADSTART));
}

// PA0 and PA1 for analog input
void adc_gpio_init(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;	// enable GPIOA
	// init PA0
	GPIOA->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1);
	// set PA0 analog mode
	GPIOA->MODER |= GPIO_MODER_MODER0 | GPIO_MODER_MODER1;

	ADC1->CHSELR = 0;	// unselect all ADC channels
}

uint32_t read_adc_channel(unsigned channel) {
	// unselect all ADC channels
	ADC1->CHSELR = 0;
	// select channel
	ADC1->CHSELR |= 1 << channel;
	uint32_t data = ADC1->DR;
	return data;
}

int main(void)
{

	for(;;);
}
