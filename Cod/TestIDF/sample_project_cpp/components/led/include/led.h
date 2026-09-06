////////////////////////// Includes ///////////////////////////////////

#include <cstdio>
#include <driver/gpio.h>
#include "pins.h"

#pragma once

/////////////// Function Declarations /////////////////////////////////

void onboard_led_init();
void onboard_led_on();
void onboard_led_off();