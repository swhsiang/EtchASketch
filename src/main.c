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

void spi1_init(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN;	// enable GPIOA and GPIOB
	GPIOA->MODER &= ~(3 << 2*15);	// initialize/declare PA15
	GPIOA->MODER |= 2 << 2*15;
	GPIOA->MODER &= ~(3 << 2*5);	// initialize/declare PA5
	GPIOA->MODER |= 2 << 2*5;
	GPIOB->MODER &= ~(3 << 2*5);	// initialize/declare PB5
	GPIOB->MODER |= 2 << 2*5;
	GPIOA->MODER &= ~(3 << 2*6);	// initialize/declare PA6
	GPIOA->MODER |= 2 << 2*6;

	GPIOA->AFR[1] &= ~(0xf<<(4*7));		// set AF0 for PA15
	GPIOA->AFR[0] &= ~(0xf<<(4*5));		// set AF0 for PA5
	GPIOB->AFR[0] &= ~(0xf<<(4*5));		// set AF0 for PB5
	GPIOA->AFR[0] &= ~(0xf<<(4*6));		// set AF0 for PA6

	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;	// enable SPI1 clock

	SPI1->CR1 &= ~SPI_CR1_BIDIMODE;	// 2-line unidirectional data mode
	SPI1->CR1 |= SPI_CR1_MSTR;	// master configuration
	SPI1->CR1 |= SPI_CR1_BR_2;	// baud rate is 1.5 MHz
	SPI1->CR1 &= ~SPI_CR1_BR_1 & ~SPI_CR1_BR_0;	// baud rate is 1.5 MHz
	SPI1->CR1 &= ~SPI_CR1_CPOL;	// clock that is low when channel is idle
	SPI1->CR1 &= ~SPI_CR1_CPHA;	// first clock transition is the first data capture edge
	SPI1->CR2 |= SPI_CR2_SSOE;	// slave select output enable

	SPI1->CR2 |= SPI_CR2_DS_3 | SPI_CR2_DS_1 | SPI_CR2_DS_0;	// data word size of 12 bits
	SPI1->CR2 &= ~SPI_CR2_DS_2;	// data word size of 12 bits

	SPI1->CR1 |= SPI_CR1_SPE;	// enable SPI1 for operation
}

int main(void)
{

	for(;;);
}
