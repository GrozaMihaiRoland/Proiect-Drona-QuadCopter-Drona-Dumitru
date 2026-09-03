#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "adc.h"
#include "motor.h"

const char* ssid = "DIGI-8ykM";
const char* password = "qrCGbvET";

IPAddress local_IP(192, 168, 100, 200);
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress laptop_IP(192, 168, 100, 176);

const uint16_t laptop_port = 7500;
const uint16_t esp32_port = 8000;

WiFiUDP udp;
Battery bat;
TwoWire wire_motor = TwoWire(0);
Motor m1, m2, m3, m4;

void setup() {
  //Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  //Serial.print("IP: "); Serial.println(WiFi.localIP());

  udp.begin(8000);
  bat.init();
  wire_motor.begin(1,2);

  m1.init(&wire_motor, M1_ADDRESS, M1_PWM_PIN);
  m2.init(&wire_motor, M2_ADDRESS, M2_PWM_PIN);
  m3.init(&wire_motor, M3_ADDRESS, M3_PWM_PIN);
  m4.init(&wire_motor, M4_ADDRESS, M4_PWM_PIN);
}

void loop() {
  uint16_t battery_voltage = bat.read_level();
  uint16_t motor_voltage = m3.read_voltage();
  uint16_t motor_current = m3.read_current();

  uint8_t len = udp.parsePacket();
  if (len == 2) {
    uint16_t duty;
    udp.read((uint8_t*)&duty, sizeof(duty));
    duty = constrain(duty, 0, 1023);
    m1.set_duty(duty);
    m2.set_duty(duty);
    m3.set_duty(duty);
    m4.set_duty(duty);
  }

  static uint32_t last_send = 0;

  if (millis() - last_send >= 50) {
      last_send = millis();

      udp.beginPacket(laptop_IP, laptop_port);
      udp.write((uint8_t*)&motor_voltage, sizeof(motor_voltage));
      udp.write((uint8_t*)&motor_current, sizeof(motor_current));
      udp.write((uint8_t*)&battery_voltage, sizeof(battery_voltage));
      udp.endPacket();
  }
}
