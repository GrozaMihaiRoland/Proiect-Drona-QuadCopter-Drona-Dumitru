#include "feedback.h"
#include "esp_log.h"

static const char* TAG = "feedback";

input_commands_t Feedback::active_remote_commands = {0, 0, 0, 0};
drone_state_t Feedback::active_state = DRONE_STATE_SETUP;
Motor Feedback::m1; 
Motor Feedback::m2;
Motor Feedback::m3;
Motor Feedback::m4;
Accelerometer Feedback::acc;
Gyroscope Feedback::gyro;
Barometer Feedback::bar;

void Feedback::set_state(drone_state_t state)
{
    active_state = state;
}

drone_state_t Feedback::get_active_state()
{
    return active_state;
}

void Feedback::initialize_hardware()
{
    i2c_motor_driver_bus_init();
    m1.init(M1_I2C_ADDRESS, MOTOR_1_PIN);
    m2.init(M2_I2C_ADDRESS, MOTOR_2_PIN);
    m3.init(M3_I2C_ADDRESS, MOTOR_3_PIN);
    m4.init(M4_I2C_ADDRESS, MOTOR_4_PIN);

    i2c_sensor_bus_init();
    acc.init();
    gyro.init();
    bar.init();
}

void Feedback::set_remote_commands(input_commands_t commands)
{
    active_remote_commands = commands;
}

void Feedback::get_active_remote_commands(input_commands_t *buf)
{
    *buf = active_remote_commands;
}