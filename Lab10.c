#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <string.h>
#include <stdlib.h> // for strtoul()

int display_initialized;

void init_usart2(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;	// enable GPIOA
	GPIOA->MODER &= ~(3 << 4);	// initialize/declare PA2
	GPIOA->MODER |= 2 << 4;		// set to general purpose output
	GPIOA->MODER &= ~(3 << 6);	// initialize/declare PA3
	GPIOA->MODER |= 2 << 6;		// set to general purpose output
	GPIOA->AFR[0] &= ~0xffff00ff;	// resets AFR for PA2 and PA3
	GPIOA->AFR[0] |= 0x1100;	// sets AFR for PA2 and PA3
	GPIOA->OSPEEDR = 0x30;	// set output speed register to high speed

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

static void nano_wait(int t)
{
    asm("       mov r0,%0\n"
        "repeat:\n"
        "       sub r0,#83\n"
        "       bgt repeat\n"
        : : "r"(t) : "r0", "cc");
}

static void cmd(char b)
{
    while((SPI2->SR & SPI_SR_TXE) != SPI_SR_TXE);
    SPI2->DR = b;
}

static void data(char b)
{
    while((SPI2->SR & SPI_SR_TXE) != SPI_SR_TXE);
    SPI2->DR = 0x200 | b;
}

void init_lcd(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	GPIOB->MODER &= ~((3<<(2*12)) | (3<<(2*13)) | (3<<(2*15)) );
	GPIOB->MODER |=   (2<<(2*12)) | (2<<(2*13)) | (2<<(2*15));
	GPIOB->AFR[1] &= ~( (0xf<<(4*(12-8))) | (0xf<<(4*(13-8))) | (0xf<<(4*(15-8))) );

	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	RCC->APB1RSTR |= RCC_APB1RSTR_SPI2RST;
	RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI2RST;

	SPI2->CR1 &= ~SPI_CR1_BR;
	SPI2->CR1 |= SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE | SPI_CR1_MSTR;
	SPI2->CR1 |= SPI_CR1_BR;
	SPI2->CR2 = SPI_CR2_SSOE | SPI_CR2_NSSP | SPI_CR2_DS_3 | SPI_CR2_DS_0;
	SPI2->CR1 |= SPI_CR1_SPE;

	nano_wait(100000000); // Give it 100ms to initialize
	cmd(0x38);  // 0011 NF00 N=1, F=0: two lines
	cmd(0x0c);  // 0000 1DCB: display on, no cursor, no blink
	cmd(0x01);  // clear entire display
	nano_wait(6200000); // clear takes 6.2ms to complete
	cmd(0x02);  // put the cursor in the home position
	cmd(0x06);  // 0000 01IS: set display to increment
}

void display1(const char *s)
{
	cmd(0x02); // put the cursor on the beginning of the first line.
	int x;
	for(x=0; x<16; x+=1)
		if (s[x])
			data(s[x]);
		else
			break;
	for(   ; x<16; x+=1)
		data(' ');
}

void display2(const char *s)
{
	cmd(0xc0); // put the cursor on the beginning of the second line.
	int x;
	for(x=0; x<16; x+=1)
		if (s[x] != '\0')
			data(s[x]);
		else
			break;
	for(   ; x<16; x+=1)
		data(' ');
}

//============================================================================
// repeat_write()
// Continually write, to the serial output, three alphabets on each line.
//
void repeat_write(void)
{
    for(;;)
    {
        int x;
        for(x=0; x<3; x++)
        {
            char a;
            for(a='A'; a<='Z'; a++)
                writechar(a);
        }
        writechar('\n');
    }
}

//============================================================================
// Continually read characters from the serial port and write them back.
void echo(void)
{
    for(;;)
        writechar(readchar());
}

//============================================================================
// print()
// Write a string to the serial port output.
// Return the length of the string.
//
int print(const char *s)
{
    const char *ptr;
    for(ptr=s; *ptr; ptr+=1)
        writechar(*ptr);
    return ptr - s;
}
int println(const char *s)
{
    int len = print(s);
    print("\n");
    return len;
}

//============================================================================
// readln()
// Read a line of input.  Echo characters.  Handle backspace and printing
// of control characters.
//
int readln(void *v, int len)
{
    int i;
    char *c = v;
    for(i=0; i<len; i++)
    {
        char tmp = readchar();
        if (tmp == '\b' || tmp == '\177')
        {
            if (i == 0)
            {
                i -= 1;
            }
            else
            {
                if (c[i-1] < 32)
                {
                    print("\b\b  \b\b");
                }
                else
                {
                    print("\b \b");
                }
                i -= 2;
            }
            continue;
        }
        else if (tmp == '\r')
        {
            print("\r\n");
            c[i++] = '\n';
            return i;
        }
        else if (tmp == 0)
        {
            print("^0");
        }
        else if (tmp == 28)
        {
            print("^\\");
        }
        else if (tmp < 32)
        {
            writechar('^');
            writechar('A'-1+tmp);
        }
        else
        {
            writechar(tmp);
        }
        c[i] = tmp;
    }
    return i;
}

//===========================================================================
// Act on a command read by testbench().
void action(char **words)
{
	int value;

    if (words[0] != 0)
    {
        if (strcasecmp(words[0],"init") == 0)
        {
            if (strcasecmp(words[1],"lcd") == 0)
            {
                init_lcd();
                display_initialized = 1;
                println("LCD initialized.");
                return;
            }
        }
        if (display_initialized == 1)
        {
			if (strcasecmp(words[0],"display1") == 0)
			{
				display1(words[1]);
				return;
			}
			if (strcasecmp(words[0],"display2") == 0)
			{
				display2(words[1]);
				return;
			}
        }

        if (strcasecmp(words[0], "set") == 0)
		{
			if (strcasecmp(words[1], "portc") == 0)
			{
				value = strtoul(words[2], 0,2);	// convert the binary string into an integer value
				RCC->AHBENR |= RCC_AHBENR_GPIOCEN;	// enable GPIOC
				GPIOC->MODER &= ~0x3ffffff;	// reset the MODER, lower 12 bits
				GPIOC->MODER |= 0x1555555;	// set the lower 12 bits to be outputs
				GPIOC->ODR |= value;	// output the given value
				return;
			}
		}

		if (strcasecmp(words[0], "clear") == 0)
		{
			if (strcasecmp(words[1], "portc") == 0)
			{
				value = strtoul(words[2], 0,2);	// convert the binary string into an integer value
				RCC->AHBENR |= RCC_AHBENR_GPIOCEN;	// enable GPIOC
				GPIOC->MODER &= ~0x3ffffff;	// reset the MODER, lower 12 bits
				GPIOC->ODR |= value;	// output the given value
				return;
			}
		}
        else if (display_initialized == 0)
        {
        	println("Warning: display not initialized yet.  Please type 'init lcd' first.");
        }
        println("Unrecognized command.");
    }
}

//===========================================================================
// Interact with the hardware.
void testbench(void)
{
    println("STM32 testbench.");
    for(;;)
    {
        char buf[60];
        print("> ");
        int sz = readln(buf, sizeof buf);
        if (sz > 0)
            buf[sz-1] = '\0';
        char *words[7] = { 0,0,0,0,0,0,0 };
        int i;
        char *cp = buf;
        for(i=0; i<6; i++)
        {
            // strtok tokenizes a string, splitting it up into words that
            // are divided by any characters in the second argument.
            words[i] = strtok(cp," \t");
            // Once strtok() is initialized with the buffer,
            // subsequent calls should be made with NULL.
            cp = 0;
            if (words[i] == 0)
                break;
            if (i==0 && strcasecmp(words[0], "display1") == 0)
            {
                words[1] = strtok(cp, ""); // words[1] is rest of string
                break;
            }
            if (i==0 && strcasecmp(words[0], "display2") == 0)
            {
                words[1] = strtok(cp, ""); // words[1] is rest of string
                break;
            }
        }
        action(words);
    }
}

int main(void)
{
    init_usart2();
    //repeat_write();
    //echo();
    testbench();
    return 0;
}
