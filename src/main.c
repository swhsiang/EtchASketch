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

// ILI9341 TFT display
// https://github.com/G6EJD/ESP32-and-how-to-use-ILI9341-TFT-Display

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

#include "display.h"
#ifndef pgm_read_byte
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif

// ADC
void adc_clock_init() {
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
void adc_gpio_init() {
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;	// enable GPIOB
	// init PB0
	GPIOB->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1);
	// set PB0 analog mode
	GPIOB->MODER |= GPIO_MODER_MODER0 | GPIO_MODER_MODER1;

	ADC1->CHSELR = 0;	// unselect all ADC channels
}

void adc_init() {
	adc_clock_init();
	adc_gpio_init();
}


uint32_t read_adc_channel(unsigned channel) {
	// unselect all ADC channels
	ADC1->CHSELR = 0;
	// select channel
	ADC1->CHSELR |= 1 << channel;
	uint32_t data = ADC1->DR;
	return data;
}


// display_data_line_init(void)
void display_data_line_init() {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;	// enable GPIOA
	// init PA0 ~ PA7
	GPIOA->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2
			| GPIO_MODER_MODER3 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6
			| GPIO_MODER_MODER7);

	// set PA0 ~ PA7 to output mode
	GPIOA->MODER |= GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2_0
			| GPIO_MODER_MODER3_0 | GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0
			| GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0;

}

// RD: PB4
// WR: PB5
// C\D: PB6
// CS: PB7
// RST: PB8
void display_control_gpio_init(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;	// enable GPIOB
	// init PB4 ~ PB8
	GPIOB->MODER &= ~( GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6| GPIO_MODER_MODER7 | GPIO_MODER_MODER8);

	// set PB4 ~ PB7 to output mode
	GPIOB->MODER |= GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0 | GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0;
}

// display
void display_gpio_init(void) {
	display_control_gpio_init();
	display_data_line_init();
}


// RD: PB4
// WR: PB5
// C\D: PB6 (command / data )
// CS: PB7
// RST: PB8
uint32_t write_process(uint8_t data) {
	uint32_t dataline = 0;
	uint32_t shift = 7;
	for (uint8_t bit = 0x80; bit; bit >>= 1) {
		if((data) & bit) {
			dataline |= GPIO_BSRR_BS_0 << shift;
		} else {
			dataline |= GPIO_BSRR_BR_0 << shift;
		}
		shift--;
	}

	return dataline;
}

// 400 is a magic number!!!!!
#define PARALLEL_DELAY 4000000
#define WRITE_DELAY    1000000

void write_command(uint32_t cmd) {
	// min: 15 ns
	GPIOA->BSRR |= write_process(cmd);
	// 0:command is selected (PB6)
	GPIOB->BSRR |= GPIO_BSRR_BS_4 | GPIO_BSRR_BR_5 | GPIO_BSRR_BR_6| GPIO_BSRR_BR_7;
	nano_wait( PARALLEL_DELAY );


	// min: 15 ns, hold time
	GPIOB->BSRR |= GPIO_BSRR_BS_5;
	nano_wait( PARALLEL_DELAY );
}

void write_data(uint32_t data) {
	// min: 15 ns
	GPIOA->BSRR |= write_process(data);
	// 1: data is selected (PB6)
	GPIOB->BSRR |= GPIO_BSRR_BR_5 | GPIO_BSRR_BS_6| GPIO_BSRR_BR_7;
	nano_wait( PARALLEL_DELAY );

	// min: 15 ns, hold time
	GPIOB->BSRR |= GPIO_BSRR_BS_5;
	nano_wait( PARALLEL_DELAY );
}

void display_init() {
    uint8_t        cmd, x, numArgs;
    const uint8_t *addr = initcmd;
    while((cmd = pgm_read_byte(addr++)) > 0) {
        //writeCommand(cmd);
    	write_command(cmd);
        x       = pgm_read_byte(addr++);
        numArgs = x & 0x7F;
        while(numArgs--) {
        	//spiWrite(pgm_read_byte(addr++));
        	write_data(pgm_read_byte(addr++));
        	//nano_wait(WRITE_DELAY);
        }
        nano_wait(WRITE_DELAY);
        //if(x & 0x80) {
        //	nano_wait(120 * 1000 * 1000);
        //}
    }

    // test
    //Display_fillScreen(ILI9341_GREEN);
}

int main(void)
{
	//adc_init();
	display_gpio_init();
	display_init();
	for(;;);
}
