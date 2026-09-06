////////////////////////// Includes ///////////////////////////////////

#include <driver/gpio.h>

#pragma once

/////////////////// Pin Definitions //////////////////////////

typedef enum
{
    // Motor PWM Pins
    MOTOR_1_PIN = GPIO_NUM_6,
    MOTOR_2_PIN = GPIO_NUM_15,
    MOTOR_3_PIN = GPIO_NUM_17,
    MOTOR_4_PIN = GPIO_NUM_8,

    // Motor driver pins
    MOTOR_I2C_SDA_PIN = GPIO_NUM_1,
    MOTOR_I2C_SCL_PIN = GPIO_NUM_2,

    // Sensor pins
    SENSOR_I2C_SDA_PIN = GPIO_NUM_11,
    SENSOR_I2C_SCL_PIN = GPIO_NUM_12,

    // ADC pins
    BATTERY_SENSE_PIN = GPIO_NUM_10,

    // led pin
    ONBOARD_LED_PIN = GPIO_NUM_37,
    
} drone_pin_t;