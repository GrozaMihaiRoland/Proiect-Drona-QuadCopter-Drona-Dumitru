// --------------------------------- Includes ----------------------------- //

#include "motor.h"
#include "adc.h"
#include <Wire.h>
#include <Wireless.h>

// ----------------------- Defines and global variables ------------------- //

typedef enum : uint8_t 
{
  ACTIVE,
  IDLE,

} motor_modes_t;

Motor m1, m2, m3, m4;
Battery bat;
TwoWire wire_motor = TwoWire(0);
Wireless wl;

motor_modes_t mode = IDLE;
int16_t current_duty = 0;

// ----------------------- Setup ------------------------- //

void setup() 
{
  //Serial.begin(115200);
  bat.init();
  wire_motor.begin(1, 2);
  wl.connect();

  m1.init(&wire_motor, M1_ADDRESS, M1_PWM_PIN);
  m2.init(&wire_motor, M2_ADDRESS, M2_PWM_PIN);
  m3.init(&wire_motor, M3_ADDRESS, M3_PWM_PIN);
  m4.init(&wire_motor, M4_ADDRESS, M4_PWM_PIN);
  delay(100);
}

// ------------------- Functions ----------------------------- //

void get_input() 
{
  wireless_command_t command = wl.get_command();
  if (command == WL_START_MOTORS) 
  {
    mode = ACTIVE;
  }
  if (command == WL_STOP_MOTORS) 
  {
    mode = IDLE;
  }
}

// -------------------- Main loop ------------------------ //

void loop() 
{
  get_input();

  switch (mode) 
  {
    case IDLE:
      current_duty = current_duty - 32;
      if (current_duty < 0) 
      {
        current_duty = 0;
      }

      m1.set_duty(current_duty); // Right Down
      m2.set_duty(0);            // Left Down 
      m3.set_duty(current_duty); // Left Up
      m4.set_duty(0);            // Right Up
      break;

    case ACTIVE:
      current_duty = current_duty + 32;
      if (current_duty > 1023) 
      {
        current_duty = 1023;
      }

      m1.set_duty(current_duty);
      m2.set_duty(0);
      m3.set_duty(current_duty);
      m4.set_duty(0);
      break;
  }

  //Serial.printf("%d mV, %d mA, %d mV bat\n", m2.read_voltage(), m2.read_current(), bat.read_level());
  wl.send_telemetry(m3.read_voltage(), m3.read_current(), bat.read_level());
  delay(30);
}