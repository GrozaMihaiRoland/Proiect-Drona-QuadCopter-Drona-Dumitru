#include "Wireless.h"

void Wireless::connect() 
{
  // configure station mode, set ip address of the device and connect to LAN
  WiFi.mode(WIFI_STA);
  WiFi.config(esp32_IP, gateway, subnet);
  //WiFi.begin("DIGI-4nRF", "9JT5484h");
  WiFi.begin("DIGI-8ykM", "qrCGbvET");

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(250);
  }

  udp.begin(esp32_port);
}

wireless_command_t Wireless::get_command()
{

  uint8_t package_len = udp.parsePacket();
  wireless_command_t command;
  
  if(package_len == 0)
  {
    return WL_NO_COMMAND;
  }

  package_len = udp.read((uint8_t*)&command, sizeof(command));
  if(package_len == 0)
  {
    return WL_NO_COMMAND;
  }

  return command;
}

void Wireless::send_telemetry(uint16_t motor_voltage, uint16_t motor_current, uint16_t battery_voltage)
{
  udp.beginPacket(laptop_IP, laptop_port);
  udp.write((uint8_t*)&motor_voltage, sizeof(motor_voltage));
  udp.write((uint8_t*)&motor_current, sizeof(motor_current));
  udp.write((uint8_t*)&battery_voltage, sizeof(battery_voltage));
  udp.endPacket();
}