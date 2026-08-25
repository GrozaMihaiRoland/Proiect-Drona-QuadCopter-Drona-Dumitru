#include "motor.h"

// sets EN_OUT bit, PMODE to PH/EN control and disables current control, initialises pwm control
uint8_t Motor::init(TwoWire* wire, motor_driver_address_t address, motor_pwm_pin_t pin)
{ 
  this->address = address;
  this->pin = pin;
  this->wire = wire;

  wire->beginTransmission(address);
  wire->write(CONFIG3_addr);
  wire->write((0 << IMODE_bit)| 
             (1 << SMODE_bit)| 
             (0 << OCP_MODE_bit)| 
             (1 << TSD_MODE_bit));
  wire->endTransmission(true);

  wire->beginTransmission(address);
  wire->write(CONFIG4_addr);
  wire->write((0 << PMODE_bit)|(1 << STALL_REP_bit));
  wire->endTransmission(true);

  wire->beginTransmission(address);
  wire->write(CONFIG0_addr);
  wire->write((1 << EN_OUT_bit)|(1 << EN_OVP_bit));
  wire->endTransmission(true);

  ledcAttach(pin, PWM_FREQ, PWM_RESOLUTION);

  return 0;
}

uint8_t Motor::set_duty(uint16_t duty_cycle){
  ledcWrite(this->pin, duty_cycle);

  return 0;
}


uint16_t Motor::read_current(){ // reads current value in mA
  uint16_t current;
  wire->beginTransmission(address);
  wire->write(STATUS2_addr);
  wire->endTransmission(false);

  wire->requestFrom(address, 1);
  
  while(!wire->available());

  current = (((uint32_t)wire->read()*4000)/192);

  return current;
}

uint16_t Motor::read_voltage(){ // reads voltage in mV
  uint16_t voltage;
  wire->beginTransmission(address);
  wire->write(STATUS1_addr);
  wire->endTransmission(false);

  wire->requestFrom(address, 1);
  
  while(!wire->available());

  voltage = (((uint32_t)(wire->read() - 2)*10813)/176);
  //voltage = wire->read();
  return voltage;
}