#include "potentiometer.h"
#include "block.h"

void adc_helper_UPDN(uint16_t new_val, Displacement *disp) {
  // This variable is static because we want to program to remember this value
  // and alter it appropriately each time the function is called
	if ((new_val < OFFSET_Y_AXIS - 1) || (new_val + OFFSET_Y_AXIS > ADC_SCALES - 1) )
		return;

	if (new_val == disp->y_disp) {
		return;
	}

	if (new_val > disp->y_disp) {
		disp->y_direction = UP;
		disp->y_diff = new_val - disp->y_disp;
	} else {
		disp->y_direction = DOWN;
		disp->y_diff = disp->y_disp - new_val;
	}


	disp->y_disp = new_val;
	disp->y_changed = CHANGED;
}

void adc_helper_RL(uint32_t new_val, Displacement *disp) {
	if ((new_val < OFFSET_X_AXIS - 1) || (new_val + OFFSET_X_AXIS > ADC_SCALES - 1) )
		return;

	if (new_val == disp->x_disp) {
		return;
	}

	if (new_val > disp->x_disp) {
		disp->x_direction = LEFT;
		disp->x_diff = new_val - disp->x_disp;
	} else {
		disp->x_direction = RIGHT;
		disp->x_diff = disp->x_disp - new_val;
	}

	disp->x_disp = new_val;
	disp->x_changed = CHANGED;
}

// for the potentiometers
void adc_init() {
  // int * block;
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;   // enable clock to Port C
  GPIOC->MODER |= 0xf;                 // set PC0 and PC1 for analog input
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
  ADC1->CFGR1 |= ADC_CFGR1_CONT | ADC_CFGR1_RES_0; /* (2) */
  // FIXME determine the sample rate of ADC
  ADC1->SMPR |=
      ADC_SMPR1_SMPR_0 | ADC_SMPR1_SMPR_1 | ADC_SMPR1_SMPR_2; /* (4) */
}

void read_adc(Displacement* disp) {
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
    adc_helper_UPDN(ADC1->DR, disp);

    // 1. CHECK IF THE DATA REGISTER HAS CHANGED BY 2^n / (ROW_NUM OR COL_NUM)
    // (THIS CAN BE IN THE POSITIVE OR NEGATIVE DIRECTIONS)
    // 2. IF IT HAS, INITIATE THE SOFTWARE TRIGGER AND CALL THE DISPLAY FUNCTION
    // 3. CALCULATE THE DISTANCE MOVED BY SUBTRACTING THE VALUE WITHIN THE DATA
    // REGISTER BY THE PREVIOUS VALUE RECORDED QUESTION: IS THIS VALUE ALWAYS
    // GOING TO BE 2^n / (ROW_NUM OR COL_NUM) AND IS THIS STEP NECESSARY?
    // 4. THE VALUE IN STEP 3 MUST BE THEN DIVIDED BY THE MINIMUM VALUE NEEDED
    // TO UPDATE THE SCREEN 2^n / (ROW_NUM OR COL_NUM) PERHAPS THE DISTANCE SENT
    // TO DAVIDS FUNCITON IS ALWAYS GOING TO BE 1. IT IS STILL NECESSARY TO FIND
    // THE DIFFERENCE BETWEEN THE DATA REGISTER AND ITS PREVIOUS VALUE
    // This function will hopefully find the difference between the data
    // register and its previous value
    ADC1->CHSELR = 0;                    // unselect all ADC channels
    ADC1->CHSELR |= ADC_CHSELR_CHSEL11;  // 1<<11;	// select channel 11
    while (!(ADC1->ISR & ADC_ISR_ADRDY))
      ;                          // wait for ADC ready
    ADC1->CR |= ADC_CR_ADSTART;  // start the ADC
    while (!(ADC1->ISR & ADC_ISR_EOC))
      ;  // wait for end of conversion
    // This function will hopefully find the difference between the data
    // register and its previous value
    adc_helper_RL(ADC1->DR, disp);
}

Displacement *create_displacement(uint16_t x_disp, uint16_t x_dir, uint16_t y_disp, uint16_t y_dir) {
	Displacement *disp = (Displacement *) malloc(sizeof(Displacement));
	disp->x_direction = x_dir;
	disp->x_disp = x_disp;
	disp->x_changed = NON_CHANGED;

	disp->y_direction = y_dir;
	disp->y_disp = y_disp;
	disp->y_changed = NON_CHANGED;
	return disp;
}
