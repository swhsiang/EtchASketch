//============================================================================
// hw9.c
// Name: Colin Nguyen
// Login: nguye207
//============================================================================
//
// All of these exercises should be completed using the Standard Peripheral
// firmware.  Use only direct I/O register manipulation to implement these
// subroutines.  (Do not use HAL functions or Standard Peripheral macros.)
// You should also assume that the system clock frequency, and the input to
// all timers, is 48 MHz.
//

// This homework will be graded by a test module.  That test module will
// also give you information on what works and what doesn't.  If you connect
// a CFAL1602 OLED LCD to SPI2 channel as you did for Lab 8, it will report
// information when the autotest() subroutine is invoked with an argument of 2.
// It will trap faults and timeouts with an argument of 1 or 2.
// It will neither trap faults nor timeouts with an argument of 0.
// An argument of 0 is good for running with a debugger.
//

// When you submit this work, you should only turn in hw9.c.

// This homework is intended to commemorate the fiftieth anniversary of the
// opening of the Eta Kappa Nu Beta chapter's lounge.

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <string.h>

void autotest(int);

//===========================================================================
// Configuration problem 1:
// Configure GPIO registers to use pins PA2 and PA3 for the USART2 TX and RX.
// Set the output speed register to high speed for TX.
void usart2_gpio_setup(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;	// enable GPIOA
	GPIOA->MODER &= ~(3 << 4);	// initialize/declare PA2
	GPIOA->MODER |= 2 << 4;		// set to general purpose output
	GPIOA->MODER &= ~(3 << 6);	// initialize/declare PA3
	GPIOA->MODER |= 2 << 6;		// set to general purpose output
	GPIOA->AFR[0] &= ~0xffff00ff;	// resets AFR for PA2 and PA3
	GPIOA->AFR[0] |= 0x1100;	// sets AFR for PA2 and PA3

	GPIOA->OSPEEDR = 0x30;	// set output speed register to high speed
}

//============================================================================
// Configuration problem 2:
// Configure USART2 for 9600 baud operation with 7 bits, even parity,
// one stop bit.  Use 16x oversampling.
// It should wait for both the transmitter and receiver to be enabled.
// (Note: I didn't find a symbolic definition for the bit 28 of the
//  USART_CR1 variable.  You might need that.)
//
void usart2_init_9600_7e1(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;	// enable USART2
	USART2->CR1 &= ~USART_CR1_UE;	// UE = 0
	USART2->CR1 &= ~USART_CR1_OVER8;	// 16x oversampling
	USART2->CR1 |= 0x10000000;	// set size to be 7-bit
	USART2->BRR = 0x1388;	// should be 9600 baud rate
	USART2->CR2 &= ~USART_CR2_STOP;	// 1 stop bit
	USART2->CR1 |= USART_CR1_PCE;	// enable parity control
	USART2->CR1 &= ~USART_CR1_PS;	// even parity
	USART2->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;	// set UE, TE, RE to 1

	while((USART2->CR1 & USART_CR1_TE) != USART_CR1_TE);	// wait for the transmitter to be enabled
	while((USART2->CR1 & USART_CR1_RE) != USART_CR1_RE);	// wait for the receiver to be enabled
}

//============================================================================
// Configuration problem 3:
// Configure USART2 for 38400 baud operation with 7 bits, odd parity,
// two stop bits.  Use 8x oversampling.
// It should wait for both the transmitter and receiver to be enabled.
// (Note: I didn't find a symbolic definition for the bit 28 of the
//  USART_CR1 variable.  You might need that.)
//
void usart2_init_38400_7o2(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;	// enable USART2
	USART2->CR1 &= ~USART_CR1_UE;	// UE = 0
	USART2->CR1 |= USART_CR1_OVER8;	// 8x oversampling
	USART2->CR1 |= 0x10000000;	// set size to be 7-bit
	USART2->BRR = 0x9c2;	// should be 38400 baud rate
	USART2->CR2 |= USART_CR2_STOP_1;	// 2 stop bits
	USART2->CR2 &= ~USART_CR2_STOP_0;	// 2 stop bits
	USART2->CR1 |= USART_CR1_PCE;	// enable parity control
	USART2->CR1 |= USART_CR1_PS;	// odd parity
	USART2->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;	// set UE, TE, RE to 1

	while((USART2->CR1 & USART_CR1_TE) != USART_CR1_TE);	// wait for the transmitter to be enabled
	while((USART2->CR1 & USART_CR1_RE) != USART_CR1_RE);	// wait for the receiver to be enabled
}

//============================================================================
// Configuration problem 4:
// Configure USART2 for 115200 baud, 8 bits, no parity, and one stop bit.
// Use 16x oversampling.
//
void usart2_init_115200_8n1(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;	// enable USART2
	USART2->CR1 &= ~USART_CR1_UE;	// UE = 0
	USART2->CR1 &= ~USART_CR1_OVER8;	// 16x oversampling
	USART2->CR1 &= ~USART_CR1_M;	// set size to be 8-bit
	USART2->CR1 &= ~(1<<28);	// set size to be 8-bit
	USART2->BRR = 0x1a1;	// should be 115200 baud rate
	USART2->CR2 &= ~USART_CR2_STOP;	// 1 stop bit
	USART2->CR1 &= ~USART_CR1_PCE;	// disable parity control so there's no parity
	USART2->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;	// set UE, TE, RE to 1
}

//============================================================================
// Configuration problem 5:
// Implement writechar so that it waits for the "transmitter empty" flag
// in the interrupt and status register of USART2 to be true.  Then it
// should output the character passed in the parameter c to the transmit
// data register of USART2.
// - writechar should not wait for the transmission to be complete.
// - writechar should check if the parameter is a newline ('\n') and first
//   send a carriage return ('\r').
//
void writechar(char c)
{
	while((USART2->ISR & USART_ISR_TXE) != USART_ISR_TXE);	// wait for the "transmitter empty" flag to be true
	if (c == '\n')	// check if newline
	{
		USART2->TDR = '\r';	// send a carriage return
		while((USART2->ISR & USART_ISR_TXE) != USART_ISR_TXE);	// wait for the "transmitter empty" flag to be true
	}
	USART2->TDR = c;	// output the character passed in the parameter c to the transmit data register of USART2
}

//============================================================================
// Configuration problem 6:
// Implement readchar in the following way:
// - If the USART2 ISR has the Overrun Error flag set, clear it by writing
//   the appropriate flag in the ICR.
// - Wait for the USART2 ISR to indicate that the read data register is
//   not empty.
// - Read a character from the USART2 read data register and return it.
//
char readchar(void)
{
	char x;
	if((USART2->ISR & USART_ISR_ORE) == USART_ISR_ORE)	// check if the Overrun Error flag is set
	{
		USART2->ICR |= USART_ICR_ORECF;	// clear the Overrun Error flag
	}
	while((USART2->ISR & USART_ISR_RXNE) != USART_ISR_RXNE);	// wait for the "read data register is not empty" flag to be true
	return x = USART2->RDR;	// read a character from the USART2 read data register and return it
}

//=============================================================================
// We provide this function for you.  You pass in a string.  It sends one
// character at a time to the USART.
//
int putstr(const char *s)
{
    int i;
    for(i=0; s[i] != '\0'; i++)
        writechar(s[i]);
    return i;
}

//=============================================================================
// We provide this function for you.  You pass a number in n and it
// writes back the 3-digit decimal representation in buf.
//
void num2str(char buf[4], int n)
{
    int i;
    if (n > 999) {
        strcpy(buf, "999");
        return;
    } else if (n < 0) {
        strcpy(buf, "---");
        return;
    } else
        strcpy(buf, "   ");
    for(i=2; ; i--) {
        int digit = n%10;
        n -= digit;
        n /= 10;
        buf[i] = '0' + digit;
        if (n == 0)
            return;
    }
}

int main(void)
{
    // Uncomment one of these when you are ready.
    // autotest(0); // no traps
    // autotest(1); // trap faults and timeouts
     autotest(2); // trap faults, timeouts, and explain problems.

    usart2_gpio_setup();
    usart2_init_9600_7e1();
    usart2_init_38400_7o2();
    usart2_init_115200_8n1();

    odyssey();

    for(;;)
    {
        putstr("Type a character: ");
        char c = readchar() & 0x7f;
        putstr("\nThat is character number ");
        char buf[4];
        num2str(buf, c);
        putstr(buf);
        putstr(".\n");
    }
    // Something else you might experiment with...
    //for(;;) { writechar(readchar()); }
    return 0;
}
