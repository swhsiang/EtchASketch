/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

// Write command !!!
// https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_SPITFT.cpp
// init the display
// https://github.com/adafruit/Adafruit_ILI9341/blob/master/Adafruit_ILI9341.cpp#L106

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

// RD: PB4
// WR: PB5
// C\D: PB6
// CS: PB7
// RST: PB8
void lcd_control_gpio_init(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;	// enable GPIOB
	// init PB4 ~ PA8
	GPIOB->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2
				| GPIO_MODER_MODER3 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6
				| GPIO_MODER_MODER7);

	// set PB4 ~ PB7 to output mode
	GPIOB->MODER |= GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0 | GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0;
}

// display
void display_gpio_init(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;	// enable GPIOA
	// init PA0 ~ PA7
	GPIOA->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2
			| GPIO_MODER_MODER3 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6
			| GPIO_MODER_MODER7);

	// set PB0 ~ PB7 to output mode
	GPIOA->MODER |= GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2_0
			| GPIO_MODER_MODER3_0 | GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0
			| GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0;
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

// PB0 and PB1 for analog input
void adc_gpio_init(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;	// enable GPIOA
	// init PA0
	GPIOB->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1);
	// set PA0 analog mode
	GPIOB->MODER |= GPIO_MODER_MODER0 | GPIO_MODER_MODER1;

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


// RD: PB4
// WR: PB5
// C\D: PB6
// CS: PB7
// RST: PB8
void enable_normal_display() {
	// control
	GPIOB->BSRR |= GPIO_BSRR_BR_6 | GPIO_BSRR_BS_4 |  GPIO_BSRR_BS_5 | GPIO_BSRR_BS_7;

	// command
	GPIOA->BSRR |= GPIO_BSRR_BS_0 | GPIO_BSRR_BS_1 | GPIO_BSRR_BR_2 | GPIO_BSRR_BR_3
			| GPIO_BSRR_BS_4 | GPIO_BSRR_BR_5 | GPIO_BSRR_BR_6 | GPIO_BSRR_BR_7;
}

void write_command(uint32_t hex_num) {
	uint32_t command = 0x00;
	int shift = 0;
	int index = 1 << shift;


	while (shift <= 7) {
		if ((hex_num & (1 << shift) ) == index) {
				command |= GPIO_BSRR_BS_0 << shift;
		} else {
				command |= GPIO_BSRR_BR_0 << shift;
		}
		shift++;
		index = index << shift;
	}

	GPIOA->BSRR |= command;
}

void enable_display() {
	// control
	GPIOB->BSRR |= GPIO_BSRR_BR_6 | GPIO_BSRR_BS_4 |  GPIO_BSRR_BS_5 | GPIO_BSRR_BS_7;
	write_command(0x29);
}


int main(void)
{
	lcd_control_gpio_init();
	display_gpio_init();
	enable_display();
	enable_normal_display();
	for(;;);
}
