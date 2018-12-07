#include "potentiometer.h"
#include "block.h"

#define max(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b;      \
  })

#define min(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a < _b ? _a : _b;      \
  })


#define SENSITIVE_FACTOR 10

void adc_helper_UPDN(uint16_t new_val, Displacement *disp) {
  // This variable is static because we want to program to remember this value
  // and alter it appropriately each time the function is called
	if ((new_val < OFFSET_Y_AXIS - 1) || (new_val + OFFSET_Y_AXIS > ADC_SCALES - 1) )
		return;

	if (new_val == disp->y_val) {
		return;
	}
	disp->y_changed = NON_CHANGED;

	if (new_val > disp->y_val) {
		disp->y_direction = UP;
		if (new_val - disp->y_val >  ADC_SCALES / SENSITIVE_FACTOR) {
			disp->y_diff = max(new_val - disp->y_val, ADC_SCALES);
			disp->y_changed = CHANGED;

			disp->y_val = new_val;
		}
	} else if (new_val < disp->y_val) {
		disp->y_direction = DOWN;
		if (disp->y_val - new_val >  ADC_SCALES / SENSITIVE_FACTOR) {
			disp->y_diff = max(disp->y_val - new_val, ADC_SCALES);
			disp->y_changed = CHANGED;
			disp->y_val = new_val;
		}
	}


}


void adc_helper_RL(uint32_t new_val, Displacement *disp) {
	if ((new_val < OFFSET_X_AXIS - 1) || (new_val + OFFSET_X_AXIS > ADC_SCALES - 1) )
		return;

	if (new_val == disp->x_val) {
		return;
	}


	/*
	if ((new_val < disp->x_disp + SENSITIVE_FACTOR * ADC_SCALES / COLUMN_NUM)
			|| (new_val + SENSITIVE_FACTOR * ADC_SCALES / COLUMN_NUM > disp->x_disp)) {
		return;
	}
	*/


	if (new_val > disp->x_val) {
		disp->x_direction = LEFT;
		disp->x_diff = min((new_val - disp->x_val) , ADC_SCALES);
		if (disp->x_diff >  ADC_SCALES / SENSITIVE_FACTOR) {
			disp->x_changed = CHANGED;
		}
		disp->x_val = new_val;

	} else if (new_val < disp->x_val) {
		disp->x_direction = RIGHT;
		disp->x_diff = disp->x_val - new_val;
		if (disp->x_diff >  ADC_SCALES / SENSITIVE_FACTOR) {
			disp->x_changed = CHANGED;
		}

		disp->x_val = new_val;
	}

}

// for the potentiometers
void adc_init() {
  // int * block;
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;   // enable clock to Port C
  GPIOC->MODER |= GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5;

  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;  // enable clock to ADC unit
  RCC->CR2 |= RCC_CR2_HSI14ON;         // turn on hi-spd internal 14 MHz clock
  while (!(RCC->CR2 & RCC_CR2_HSI14RDY))
    ;                       // wait for 14 MHz clock to be ready
  ADC1->CR |= ADC_CR_ADEN;  // enable ADC
  while (!(ADC1->ISR & ADC_ISR_ADRDY))
    ;  // wait for ADC to be ready
  while ((ADC1->CR & ADC_CR_ADSTART))
    ;  // wait for ADCstart to be 0
  // Change the resolution to 10 instead of 12;
  ADC1->CFGR1 |= ADC_CFGR1_CONT ; /* (2) */
  ADC1->SMPR |=
      ADC_SMPR1_SMPR_0 | ADC_SMPR1_SMPR_1 | ADC_SMPR1_SMPR_2; /* (4) */
}

void read_adc(Block* block) {
	// unselect all ADC channels
    ADC1->CHSELR = 0;
    // 1<<10;	// select channel 10
    ADC1->CHSELR |= ADC_CHSELR_CHSEL10;
    // wait for ADC ready
    while (!(ADC1->ISR & ADC_ISR_ADRDY));
    // start the ADC
    ADC1->CR |= ADC_CR_ADSTART;
    // wait for end of conversion
    while (!(ADC1->ISR & ADC_ISR_EOC));
    delay_cycles(2000);
    block->color = (ADC1->DR) % (0xFFFF);

    ADC1->CHSELR = 0;
    ADC1->CHSELR |= ADC_CHSELR_CHSEL11;
    while (!(ADC1->ISR & ADC_ISR_ADRDY));
    ADC1->CR |= ADC_CR_ADSTART;
    while (!(ADC1->ISR & ADC_ISR_EOC));
    delay_cycles(2000);
    uint16_t temp = ADC1->DR;
    block->color = (block->color * temp ) % (0xFFFF);


    ADC1->CHSELR = 0;
    ADC1->CHSELR |= ADC_CHSELR_CHSEL14;
    while (!(ADC1->ISR & ADC_ISR_ADRDY));
    ADC1->CR |= ADC_CR_ADSTART;
    while (!(ADC1->ISR & ADC_ISR_EOC));
    delay_cycles(2000);
    block->block_x = ADC1->DR / 17 ;

    ADC1->CHSELR = 0;
    ADC1->CHSELR |= ADC_CHSELR_CHSEL15;
    while (!(ADC1->ISR & ADC_ISR_ADRDY));
    ADC1->CR |= ADC_CR_ADSTART;
    while (!(ADC1->ISR & ADC_ISR_EOC));
    delay_cycles(2000);
    block->block_y = ADC1->DR / 8;
}

void displacement_init(Displacement *disp) {
	// unselect all ADC channels
    ADC1->CHSELR = 0;
    // 1<<10;	// select channel 10
    ADC1->CHSELR |= ADC_CHSELR_CHSEL10;
    // wait for ADC ready
    while (!(ADC1->ISR & ADC_ISR_ADRDY));
    // start the ADC
    ADC1->CR |= ADC_CR_ADSTART;
    // wait for end of conversion
    while (!(ADC1->ISR & ADC_ISR_EOC));

    disp->y_val = ADC1->DR;


    ADC1->CHSELR = 0;                    // unselect all ADC channels
    ADC1->CHSELR |= ADC_CHSELR_CHSEL11;  // 1<<11;	// select channel 11
    while (!(ADC1->ISR & ADC_ISR_ADRDY))
      ;                          // wait for ADC ready
    ADC1->CR |= ADC_CR_ADSTART;  // start the ADC
    while (!(ADC1->ISR & ADC_ISR_EOC))
    disp->x_val = ADC1->DR;
}

Displacement *create_displacement(uint16_t x_dir, uint16_t y_dir) {
	Displacement *disp = (Displacement *) malloc(sizeof(Displacement));
	disp->x_direction = x_dir;
	disp->x_changed = NON_CHANGED;

	disp->y_direction = y_dir;
	disp->y_changed = NON_CHANGED;

	displacement_init(disp);
	return disp;
}
