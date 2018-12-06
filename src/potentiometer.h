#include "stm32f0_discovery.h"
#include "stm32f0xx.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

uint32_t adc_helper_UPDN(uint32_t dr_check, uint32_t div);

uint32_t adc_helper_RL(uint32_t dr_check, uint32_t div);

void adc_init(void);
