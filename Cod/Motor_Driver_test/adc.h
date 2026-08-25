#pragma once
#include <Arduino.h> 
#define BAT_SENSE_PIN 10

class Battery {
  private:
  uint16_t level;

  public:
    void init();
    uint16_t read_level();
};