#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

void nano_wait(int t)
{
    asm("       mov r0,%0\n"
        "repeat:\n"
        "       sub r0,#83\n"
        "       bgt repeat\n"
        : : "r"(t) : "r0", "cc");
}

static short dispmem[] =
{
        0x002,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x0c0,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
};


//===========================================================================
// Send a command to the LCD.
void cmd(char b)
{
    while((SPI2->SR & SPI_SR_TXE) != SPI_SR_TXE);
    SPI2->DR = b;
}

//===========================================================================
// Send a character to the LCD.
void data(char b)
{
    while((SPI2->SR & SPI_SR_TXE) != SPI_SR_TXE);
    SPI2->DR = 0x200 | b;
}

//===========================================================================
// Initialize the LCD.
void init_lcd_spi(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	GPIOB->MODER &= ~(3 << 30);	// initialize/declare PB15
	GPIOB->MODER |= 2 << 30;
	GPIOB->MODER &= ~(3 << 26);	// initialize/declare PB13
	GPIOB->MODER |= 2 << 26;
	GPIOB->MODER &= ~(3 << 24);	// initialize/declare PB12
	GPIOB->MODER |= 2 << 24;
	GPIOB->AFR[1] &= ~0x0f00ffff;		// alternate function
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;


	SPI2->CR2 |= SPI_CR2_TXDMAEN;
	SPI2->CR1 |= 0xc03c;
	SPI2->CR2 = SPI_CR2_SSOE | SPI_CR2_NSSP | SPI_CR2_DS_3 | SPI_CR2_DS_0;
	SPI2->CR2 |= SPI_CR2_TXDMAEN;
	SPI2->CR1 |= SPI_CR1_SPE;



	nano_wait(100000000); // 100ms to initialize
	cmd(0x38); // 0011 NF00 N=1, F=0: two lines
	cmd(0x0c); // 0000 1DCB: display on, no cursor, no blink
	cmd(0x01); // clear entire display
	nano_wait(6200000); // 6.2ms to clear
	cmd(0x02); // put cursor in the home position
	cmd(0x06); // 0000 01IS: set display to increment
}



//===========================================================================
// Initialize the LCD to use circular DMA to write to the SPI channel.
void init_lcd(void)
{
	init_lcd_spi();

	RCC->AHBENR |= RCC_AHBENR_DMA1EN;	// enable the peripheral clock on DMA
	DMA1_Channel5->CCR &= ~DMA_CCR_EN;
	DMA1_Channel5->CPAR = (uint32_t) (&(SPI2->DR));	// configure peripheral data register
	DMA1_Channel5->CMAR =  (uint32_t) dispmem;	// configure the memory address
	DMA1_Channel5->CNDTR = sizeof(dispmem) / 2;	// configure the number of DMA transfer to be performed on the channel
	DMA1_Channel5->CCR |= DMA_CCR_DIR;	// read from memory
	DMA1_Channel5->CCR &= ~DMA_CCR_HTIE;
	DMA1_Channel5->CCR |= DMA_CCR_MSIZE_0;	// bit size
	DMA1_Channel5->CCR |= DMA_CCR_PSIZE_0;
	DMA1_Channel5->CCR |= DMA_CCR_MINC;	// configure the increment for the memory

	DMA1_Channel5->CCR &= ~DMA_CCR_PINC;
	DMA1_Channel5->CCR &= ~DMA_CCR_MEM2MEM;
	DMA1_Channel5->CCR |= DMA_CCR_CIRC;	// enabling circular mode
	DMA1_Channel5->CCR &= ~(DMA_CCR_PL);	// priority set to low
	//SPI2 -> CR2 |= SPI_CR2_TXDMAEN;
	DMA1_Channel5->CCR |= DMA_CCR_EN;	// enable the DMA channel
}



//===========================================================================
// Display a string on line 1 by writing to SPI directly.
void display1_spi(const char *s)
{
    cmd(0x02);
    int x;
    for(x=0; x<16; x+=1)
        if (s[x])
            data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        data(' ');
}



//===========================================================================
// Display a string on line 2 by writing to SPI directly.
void display2_spi(const char *s)
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



//===========================================================================
// Display a string on line 1 by using DMA to write to SPI.
void display1_dma(const char *s)
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;	// enable the peripheral clock on DMA
	DMA1_Channel5->CCR &= ~DMA_CCR_EN;	// make sure DMA is turned off
	DMA1_Channel5->CPAR = (uint32_t) (&(SPI2->DR));	// configure peripheral data register
	DMA1_Channel5->CMAR = (uint32_t) dispmem;	// configure the memory address
	DMA1_Channel5->CNDTR = sizeof(dispmem) / sizeof(dispmem[0]);	// configure the number of DMA transfer to be performed on the channel
	DMA1_Channel5->CCR |= DMA_CCR_DIR;	// read from memory

	DMA1_Channel5->CCR &= ~DMA_CCR_HTIE;
	DMA1_Channel5->CCR |=  DMA_CCR_MSIZE_0;	// bit size
	DMA1_Channel5->CCR |=  DMA_CCR_PSIZE_0;

	DMA1_Channel5->CCR |= DMA_CCR_MINC;	// configure the increment for the memory
	DMA1_Channel5->CCR &= ~(DMA_CCR_PINC);	// the peripheral should not be incremented

	DMA1_Channel5->CCR &= ~(DMA_CCR_CIRC);	// disabling circular mode
	DMA1_Channel5->CCR &= ~(DMA_CCR_MEM2MEM);	// disabling memory-to-memory
	SPI2->CR2 |= SPI_CR2_TXDMAEN;
	DMA1_Channel5->CCR &= ~(DMA_CCR_PL);	// priority set to low
	DMA1_Channel5->CCR |= DMA_CCR_EN;	// enable the DMA channel

	cmd(0x002);
	int x;

	for(x = 0; x < 16; x++)
	{
		if(s[x])
		{
			dispmem[x+1] = s[x] | 0x200;
		}
		else
		{
			break;
		}
	}
	for( ; x< 16; x++)
	{
		dispmem[x+1] = 0x220;
	}
}



//===========================================================================
// Display a string on line 2 by using DMA to write to SPI.
void display2_dma(const char *s)
{

}



//===========================================================================

// Display a string on line 1 by writing to the DMA source.
void display1(const char *s)
{
	cmd(0x20);
	int x;
	for (x=0; x<16; x++)
	{
		if(s[x])
		{
			dispmem[x + 1] = s[x] | 0x200;
		}
		else
		{
			break;
		}
	}
	for(   ; x<16; x+=1)
	{
			  dispmem[x+1] = 0x220;
	}
}



//===========================================================================

// Display a string on line 2 by writing to the DMA source.
void display2(const char *s)
{
	cmd(0x20);
	int x;

	for (x=0; x<16; x++)
	{
		if (s[x] != '\0')
		{
			dispmem[x+18] = s[x] | 0x200;
		}
		else
		{
			break;
		}
	}

	for(   ; x<16; x+=1)
	{
		dispmem[x+18] = 0x220;
	}
}


void TIM2_IRQHandler(void)
{
	static int seconds = 0;
	static int minutes = 0;
	static int hours = 0;
	seconds += 1;
	if (seconds > 59)
	{
		seconds = 0;
		minutes += 1;
	}
	if (minutes > 59)
	{
		minutes = 0;
		hours += 1;
	}
	char line[20];
	sprintf(line, "Waiting %02d:%02d:%02d", hours, minutes, seconds);
	display1(line);

	TIM2->SR &= ~TIM_SR_UIF;	// clear the Update Interrupt Flag
}

void init_tim2(void)
{
	// enable the system clock for timer 2
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	// set PSC and ARR
	TIM2->PSC = 48000 - 1;
	TIM2->ARR = 1000 - 1;

	// enable & setup other registers
	TIM2->CR1 |= TIM_CR1_DIR;
	TIM2->DIER |= TIM_DIER_UIE;

	// interrupt stuff
	NVIC->ISER[0] = 1 << TIM2_IRQn;
	TIM2->CR1 |= TIM_CR1_CEN;
}
