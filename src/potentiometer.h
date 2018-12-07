#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f0_discovery.h"
#include "block.h"
#include "stm32f0xx.h"

#ifndef _ADC_INIT_
#define _ADC_INIT_ 1
#define CHANNEL_Y_AXIS 10
#define CHANNEL_X_AXIS 11
#define OFFSET_X_AXIS ((uint16_t) 128 )
#define OFFSET_Y_AXIS ((uint16_t) 128 )
#define ADC_SCALES    ((uint16_t) 1024)
#define CHANGED 1
#define NON_CHANGED 0
#endif

typedef struct Displacement  {
	uint8_t x_changed, y_changed;
	uint32_t x_val, y_val;
	uint32_t x_diff, y_diff;
	uint8_t  x_direction, y_direction;
} Displacement;

void adc_helper_UPDN(uint16_t new_val, Displacement *disp);
void adc_helper_RL(uint32_t new_val, Displacement *disp);

void adc_init();
void read_adc(Block* block);
Displacement *create_displacement(uint16_t x_dir, uint16_t y_dir);
