#include "feedback.h"
#include "esp_log.h"

static const char* TAG = "feedback";

input_commands_t Feedback::active_remote_commands = {0, 0, 0, 0};
drone_state_t Feedback::active_state = DRONE_STATE_IDLE;
duty_commands_t Feedback::active_duty;
Motor Feedback::m1; 
Motor Feedback::m2;
Motor Feedback::m3;
Motor Feedback::m4;
Accelerometer Feedback::acc;
Gyroscope Feedback::gyro;
Barometer Feedback::bar;

drone_state_t Feedback::get_active_state()
{
    return active_state;
}

void Feedback::initialize_hardware()
{
    onboard_led_init();
    battery_init();
    i2c_motor_driver_bus_init();
    m1.init(M1_I2C_ADDRESS, MOTOR_1_PIN);
    m2.init(M2_I2C_ADDRESS, MOTOR_2_PIN);
    m3.init(M3_I2C_ADDRESS, MOTOR_3_PIN);
    m4.init(M4_I2C_ADDRESS, MOTOR_4_PIN);

    i2c_sensor_bus_init();
    acc.init();
    gyro.init();
    bar.init();
    bar.set_height_origin();
}

void Feedback::set_active_commands(input_commands_t commands)
{
    active_remote_commands = commands;
}

void Feedback::get_active_commands(input_commands_t *buf)
{
    *buf = active_remote_commands;
}

void Feedback::estimate_angles()
{
    // Temporary implementation
}

void Feedback::send_motor_commands(duty_commands_t duty)
{
    if(active_state == DRONE_STATE_ACTIVE)
    {
        active_duty = duty;
        m1.set_duty(duty.M1);
        m2.set_duty(duty.M2);
        m3.set_duty(duty.M3);
        m4.set_duty(duty.M4);
    } 
    else
    {
        ESP_LOGI(TAG, "Drone is currently idle");
    }

}

void Feedback::turn_off_motors()
{
    m1.set_duty(0.0f);
    m2.set_duty(0.0f);
    m3.set_duty(0.0f);
    m4.set_duty(0.0f);    
}

void Feedback::get_active_duty(duty_commands_t *buf)
{
    *buf = active_duty;
}

void Feedback::set_state(drone_state_t state)
{
    active_state = state;
    if (state == DRONE_STATE_IDLE)
    {
        turn_off_motors();
    }
}