#include <driver/ledc.h>
#include <driver/i2c_master.h>
#include "pins.h"

#pragma once

typedef enum : uint8_t
{
    M1_I2C_ADDRESS = 0x30,
    M2_I2C_ADDRESS = 0x31,
    M3_I2C_ADDRESS = 0x33,
    M4_I2C_ADDRESS = 0x35,

} motor_driver_i2c_address_t;

void i2c_motor_driver_bus_init();

class Motor 
{
    private:
    i2c_master_dev_handle_t handle;
    i2c_device_config_t slave_cfg;

    motor_driver_i2c_address_t address;
    drone_pin_t pwm_pin; 
    ledc_channel_t channel;
    uint32_t hpoint;

    public:
    void init(motor_driver_i2c_address_t address, drone_pin_t pwm_pin);
    void set_duty(float duty);
    float read_current();
    float read_voltage();
    
};