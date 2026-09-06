////////////////////////// Includes ///////////////////////////////////

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

typedef enum : uint8_t
{
    DRONE_HEADER_REMOTE_COMMANDS = 0x10,
    DRONE_HEADER_EMERGENCY_STOP = 0x20,
    DRONE_HEADER_LAND   = 0x21,
    DRONE_HEADER_HOVER = 0x22,
    DRONE_HEADER_WATCH_DOG = 0x00

} drone_header_t;

typedef struct
{
    drone_header_t header;
    uint8_t payload[4];

} drone_packet_t;

/////////////// Function Declarations /////////////////////////////////

void wireless_init();
void wireless_read();
void wireless_transmit();