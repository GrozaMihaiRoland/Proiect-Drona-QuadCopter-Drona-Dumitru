#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include "motors.h"
#include "battery.h"
#include "led.h"
#include "sensors.h"

#pragma once 

typedef struct
{
    uint8_t thrust;
    int8_t pitch;
    int8_t roll;
    int8_t yaw;

} input_commands_t;

typedef enum
{
    DRONE_STATE_IDLE,
    DRONE_STATE_STABILIZING,

} drone_state_t;

class Feedback
{
    private:
    static input_commands_t active_remote_commands;
    static drone_state_t active_state;

    static Motor m1, m2, m3, m4;
    static Accelerometer acc;
    static Gyroscope gyro;
    static Barometer bar;

    public:
    // Interface functions for communication
    static void set_remote_commands(input_commands_t commands);
    static void get_active_remote_commands(input_commands_t* buf);
    
    static void set_state(drone_state_t state);
    static drone_state_t get_active_state();

    // Drone flow functions
    static void initialize_hardware();  // Sets up motors and sensors 
    static void estimate_angles();      // Reads and runs angle data through a kalman filter
    static void estimate_z_pos();       // Reads and runs Z data through a kalman filter
    static void calculate_commands();   // Runs a PID algorithm to calculate commands for angle and Z stabilization
    static void send_motor_commands()         // Sends calculated duty-cycles to motors
};