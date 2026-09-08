#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "feedback.h"
#include "wireless.h"
#include "esp_log.h"

static const char* TAG = "main";

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
    Feedback::initialize_hardware();
    wireless_init();

    xTaskCreatePinnedToCore(taskTransmit, "taskTransmit", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(taskRead, "taskRead", 2048, NULL, 1, NULL, 0);
}
