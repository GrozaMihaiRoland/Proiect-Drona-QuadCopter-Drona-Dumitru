#include "adc.h"

void Battery::init(){
  pinMode(BAT_SENSE_PIN, INPUT);
}

uint16_t Battery::read_level(){
  uint16_t raw_level = ((uint32_t)analogRead(BAT_SENSE_PIN)*6840)/4095;
  level = raw_level;
  return level;
}