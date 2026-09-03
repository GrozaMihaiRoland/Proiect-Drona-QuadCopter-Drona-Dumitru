#pragma once

#include <WiFi.h>
#include <WiFiUdp.h>

typedef enum : uint8_t
{
  WL_NO_COMMAND = 0,
  WL_START_MOTORS = 1,
  WL_STOP_MOTORS = 2,

} wireless_command_t;

class Wireless
{
  private:
    WiFiUDP udp;
    IPAddress esp32_IP(192, 168, 100, 200);
    IPAddress gateway(192, 168, 100, 1);
    IPAddress subnet(255, 255, 255, 0);

    //IPAddress laptop_IP{192, 168, 100, 47}; // acasa
    //IPAddress laptop_IP{192, 168, 100, 193}; // la dumitru
    IPAddress laptop_IP(192, 168, 100, 176); // la dumitru

    uint16_t laptop_port = 7500;

    uint16_t esp32_port = 8000;

  public:
    void connect();
    wireless_command_t get_command();
    void send_telemetry(uint16_t motor_voltage, uint16_t motor_current, uint16_t battery_voltage);
};


