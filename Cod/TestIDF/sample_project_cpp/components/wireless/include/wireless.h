////////////////////////// Includes ///////////////////////////////////

#include <freertos/FreeRTOS.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <nvs_flash.h>
#include <esp_event.h>
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "feedback.h"
#include "led.h"

#pragma once

///////////////////////////////////////////////////////////////////////
/// @brief  Standard wireless packet for UDP communication
///////////////////////////////////////////////////////////////////////

typedef enum : uint32_t
{
    // Connection integrity check command header  
    COMMAND_HEADER_WATCH_DOG =                0x00,

    // Change active mode headers
    COMMAND_HEADER_SET_IDLE =                 0x21,
    COMMAND_HEADER_SET_ACTIVE =               0x22,

    // Set parameter command headers
    COMMAND_HEADER_SET_DUTY =                 0x10,
    COMMAND_HEADER_SET_COMMANDS =             0x11,

    // Transmit mode command headers
    COMMAND_HEADER_TRANSMIT_MODE_IDLE =       0x30,
    COMMAND_HEADER_TRANSMIT_MODE_TELEMETRY =  0x31,
    COMMAND_HEADER_TRANSMIT_MODE_DUTY =       0x32,
    COMMAND_HEADER_TRANSMIT_MODE_VOLTAGE =    0x33,

} drone_header_t;

typedef struct
{
    drone_header_t header;
    uint32_t payload[4];

} drone_packet_t;

/////////////// Function Declarations /////////////////////////////////

void wireless_init();
void wireless_read();
void wireless_transmit();