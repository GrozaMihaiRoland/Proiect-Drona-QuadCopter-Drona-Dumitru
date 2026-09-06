////////////////////////// Includes ///////////////////////////////////
#include "led.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "pins.h"

#pragma once

#define VOLTAGE_DIVIDER_RATIO 2
#define VOLTAGE_OFFSET 65 // in mV, calibrated with multimeter

/////////////// Function Declarations /////////////////////////////////

void battery_init();
int battery_read();