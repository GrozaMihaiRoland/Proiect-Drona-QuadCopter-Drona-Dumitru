#pragma once
#include <Wire.h>

#define CONFIG0_addr 0x09
#define CONFIG3_addr 0x0C
#define CONFIG4_addr 0x0D
#define STATUS1_addr 0x04
#define STATUS2_addr 0x05

#define EN_OUT_bit 7
#define EN_OVP_bit 6
#define IMODE_bit 6
#define PMODE_bit 3
#define SMODE_bit 5
#define OCP_MODE_bit 1
#define TSD_MODE_bit 0
#define STALL_REP_bit 5

#define PWM_FREQ 20000
#define PWM_RESOLUTION 10


typedef enum
{
    M1_ADDRESS = 0x30,
    M2_ADDRESS = 0x31,
    M3_ADDRESS = 0x33,
    M4_ADDRESS = 0x35

} motor_driver_address_t;

typedef enum
{
    M1_PWM_PIN = 6,
    M2_PWM_PIN = 15,
    M3_PWM_PIN = 17,
    M4_PWM_PIN = 8

} motor_pwm_pin_t;

class Motor 
{
  private:
    motor_driver_address_t address;
    TwoWire* wire;
    motor_pwm_pin_t pin;

  public:
    uint8_t init(TwoWire *wire, motor_driver_address_t address, motor_pwm_pin_t pin);
    uint8_t set_duty(uint16_t duty_cycle);
    uint16_t read_current();
    uint16_t read_voltage();
};
