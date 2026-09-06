#include <cstdio>
#include <freertos/FreeRTOS.h>
#include "wireless.h"
#include "led.h"
#include "battery.h"
#include "sensors.h"
#include "motors.h"
#include "esp_log.h"

static const char* TAG = "main";

void runMotors()
{
    i2c_motor_driver_bus_init();
    Motor m1;
    m1.init(M1_I2C_ADDRESS, MOTOR_1_PIN);
    
    for(float dt = 0.0f; dt < 1.0f; dt = dt + 0.01f)
    {
        m1.set_duty(dt);
        ESP_LOGI(TAG, "Current: %.1f mA, Voltage: %.1f mV, Bat Voltage: %d", m1.read_current(), m1.read_voltage(), battery_read());
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    for(uint8_t i = 0; i < 200; i++)
    {
        ESP_LOGI(TAG, "Current: %.1f mA, Voltage: %.1f mV, Bat Voltage: %d", m1.read_current(), m1.read_voltage(), battery_read());
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }    

    for(float dt = 1.0f; dt >= 0.0f; dt = dt - 0.01f)
    {
        m1.set_duty(dt);
        ESP_LOGI(TAG, "Current: %.1f mA, Voltage: %.1f mV, Bat Voltage: %d", m1.read_current(), m1.read_voltage(), battery_read());
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void taskTransmit(void *pvParameters)
{
    while(1)
    {
        wireless_transmit();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}

void taskRead(void *pvParameters)
{
    while(1)
    {
        wireless_read();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

extern "C" void app_main(void)
{
    onboard_led_init();
    wireless_init();
    battery_init();

    i2c_sensor_bus_init();
    Barometer BMP388;
    BMP388.init();

    Accelerometer BMI088_Acc;
    BMI088_Acc.init();
    
    Gyroscope BMI088_Gyro;
    BMI088_Gyro.init();

    BMP388.set_height_origin();

    xTaskCreatePinnedToCore(taskTransmit, "taskTransmit", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(taskRead, "taskRead", 2048, NULL, 1, NULL, 0);
}
