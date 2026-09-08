#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include "motors.h"
#include "battery.h"
#include "led.h"
#include "sensors.h"

#pragma once 

/////////////////////// Structs and Enums /////////////////////////////

///////////////////////////////////////////////////////////////////////
/// @brief Drone movement command data-structure
///////////////////////////////////////////////////////////////////////
typedef struct
{
    float z_height;
    float pitch;
    float roll;

} input_commands_t;

///////////////////////////////////////////////////////////////////////
/// @brief Drone duty cycle data-structure
///////////////////////////////////////////////////////////////////////
typedef struct
{
    float M1;
    float M2;
    float M3;
    float M4;

} duty_commands_t;

///////////////////////////////////////////////////////////////////////
/// @brief Drone functioning states
///////////////////////////////////////////////////////////////////////
typedef enum
{
    DRONE_STATE_IDLE,   // Motors inactive
    DRONE_STATE_ACTIVE, // Stabilization algorithm active, motors active

} drone_state_t;


////////////////////////// Classes //////////////////////////////////// 

///////////////////////////////////////////////////////////////////////
/// @brief Class that handles the drone feedback loop
///////////////////////////////////////////////////////////////////////
class Feedback
{
    private:
    static input_commands_t active_remote_commands;
    static drone_state_t active_state;
    static duty_commands_t active_duty;

    static Motor m1, m2, m3, m4;
    static Accelerometer acc;
    static Gyroscope gyro;
    static Barometer bar;

    public:
    // Interface functions for communication
    static void set_active_commands(input_commands_t commands);
    static void get_active_commands(input_commands_t* buf);
    static void get_active_duty(duty_commands_t* buf);
    static void set_state(drone_state_t state);
    static drone_state_t get_active_state();
    static void turn_off_motors();

    // Drone flow functions
    static void initialize_hardware();  // Sets up motors and sensors 
    static void estimate_angles();      // Reads and runs angle data through a kalman filter
    static void estimate_z_pos();       // Reads and runs Z data through a kalman filter
    static void calculate_commands();   // Runs a PID algorithm to calculate commands for angle and Z stabilization
    static void send_motor_commands(duty_commands_t duty);  // Sends calculated duty-cycles to motors
};