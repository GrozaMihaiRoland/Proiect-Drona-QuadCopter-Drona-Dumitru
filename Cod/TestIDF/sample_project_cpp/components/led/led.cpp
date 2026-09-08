#include "led.h"

void onboard_led_init()
{
    gpio_reset_pin((gpio_num_t)ONBOARD_LED_PIN);
    gpio_set_drive_capability((gpio_num_t)ONBOARD_LED_PIN, GPIO_DRIVE_CAP_0);
    
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE; 
    io_conf.mode = GPIO_MODE_OUTPUT;      
    io_conf.pin_bit_mask = (1ULL << ONBOARD_LED_PIN);   
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; 
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;     
    gpio_config(&io_conf); 
}

void onboard_led_on()
{
    gpio_set_level((gpio_num_t)ONBOARD_LED_PIN, 1);
}

void onboard_led_off()
{
    gpio_set_level((gpio_num_t)ONBOARD_LED_PIN, 0);
}