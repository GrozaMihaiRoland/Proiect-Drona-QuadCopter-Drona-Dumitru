
// --------------------------------- Includes ----------------------------- //

/*#include "motor.h"
#include "adc.h"
#include <Wire.h>
#include "bmi088.h"
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include "KalmanFilter.h"
#include "Wireless.h"

#define acc_adr 0x18
#define gyro_adr 0x69
#define BMP_SDA 11
#define BMP_SCL 12
#define ADDR 0x77
#define SEALEVELPRESSURE_HPA (1013.25)

// ----------------------- Defines and global variables ------------------- //

typedef enum : uint8_t 
{
  ACTIVE,
  IDLE,

} motor_modes_t;

Motor m1, m2, m3, m4;
Battery bat;
TwoWire wire_motor = TwoWire(0);
Adafruit_BMP3XX bmp;
BMI088 imu = BMI088(gyro_adr, acc_adr);
Filter2D filter = Filter2D();
vector accData;
Wireless wl;

motor_modes_t mode = IDLE;
int16_t current_duty = 0;

unsigned long lTime, cTime;

float sigma_acc = 0.1f;
float sigma_alt = 0.1f;

BLA::Matrix<2,2,float> F;
BLA::Matrix<2,1,float> B;
BLA::Matrix<1,2,float> H = {1.0f, 0.0f};
BLA::Matrix<1,1,float> R = {sigma_alt * sigma_alt}; 
BLA::Matrix<2,2,float> Q;

// ----------------------- Setup ------------------------- //

void setup() 
{
  Serial.begin(115200);
  bat.init();
  wire_motor.begin(1, 2);
  wl.connect();

  Serial.print("WiFi connected, IP: ");
  Serial.println(WiFi.localIP());

  m1.init(&wire_motor, M1_ADDRESS, M1_PWM_PIN);
  m2.init(&wire_motor, M2_ADDRESS, M2_PWM_PIN);
  m3.init(&wire_motor, M3_ADDRESS, M3_PWM_PIN);
  m4.init(&wire_motor, M4_ADDRESS, M4_PWM_PIN);

  Wire.begin(BMP_SDA, BMP_SCL);
  imu.setMeas();

  if (!bmp.begin_I2C(ADDR)) {   // hardware I2C mode, can pass in address & alt Wire
  //if (! bmp.begin_SPI(BMP_CS)) {  // hardware SPI mode  
  //if (! bmp.begin_SPI(BMP_CS, BMP_SCK, BMP_MISO, BMP_MOSI)) {  // software SPI mode
    Serial.println("Could not find a valid BMP3 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_2X);
  bmp.setTemperatureOversampling(BMP3_NO_OVERSAMPLING);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_100_HZ);

  lTime = millis();
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
}*/

// -------------------- Main loop ------------------------ //

/*void loop() 
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

  cTime = millis();
  if (cTime- lTime > 10) {
    float dt = (cTime - lTime) / 1000.0f;

    B = {0.5f * dt * dt, dt};
    F = {1.0f, dt, 0.0f, 1.0f};
    Q = {0.25f * dt * dt * dt * dt * sigma_acc * sigma_acc, 0.5f * dt * dt * dt * sigma_acc * sigma_acc, 0.5f * dt * dt * dt * sigma_acc * sigma_acc, dt * dt * sigma_acc * sigma_acc};

    if (!bmp.performReading()) {
      Serial.println("Failed to perform reading :(");
    }

    float alt = bmp.readAltitude(SEALEVELPRESSURE_HPA);

    imu.readAcc();
    accData = imu.getAcc();

    filter.get_measurement(alt);
    filter.get_command(accData.z - 9.81f);
    filter.estimate(F,B,Q);
    filter.update(H,R);
    BLA::Matrix<2,1,float> x = filter.get_state();
    wl.send_telemetry(m3.read_voltage(), m3.read_current(), bat.read_level(), x(0,0));

    lTime = cTime;
  }
  //Serial.printf("%d mV, %d mA, %d mV bat\n", m2.read_voltage(), m2.read_current(), bat.read_level());
}*/

#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include "bmi088.h"
#include "KalmanFilter.h"
#include "adc.h"

#define BMP_SDA 11
#define BMP_SCL 12
#define ADDR 0x77

const char* ssid = "DIGI-8ykM";
const char* password = "qrCGbvET";

IPAddress local_IP(192, 168, 100, 200);
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress laptop_IP(192, 168, 100, 176);

const uint16_t udp_port = 7500;

WiFiUDP udp;
Adafruit_BMP3XX bmp;
Battery bat;
BMI088 imu = BMI088(gyro_adr, acc_adr);
Filter2D filter = Filter2D();
vector accData, gyroData;

float pitch, roll;
float P_pitch, P_roll;

float alt;
float sigma_acc = 5.0f;
float sigma_alt = 5.0f;
float std_pitch = 10.0f;
float std_roll = 10.0f;
float dt = 0.01f;

BLA::Matrix<2,2,float> F;
BLA::Matrix<2,1,float> B;
BLA::Matrix<1,2,float> H = {1.0f, 0.0f};
BLA::Matrix<1,1,float> R = {sigma_alt * sigma_alt}; 
BLA::Matrix<2,2,float> Q;

void setup() {
  //Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  //Serial.print("IP: "); Serial.println(WiFi.localIP());

  udp.begin(8000);
  bat.init();

  Wire.begin(BMP_SDA, BMP_SCL);
  imu.setMeas();

  if (!bmp.begin_I2C(ADDR)) {
    Serial.println("BMP388 not found");
    while (1);
  }
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setTemperatureOversampling(BMP3_NO_OVERSAMPLING);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);
}

void loop() {
  if (!bmp.performReading()) return;
  alt = bmp.readAltitude(1013.25);
  uint16_t bat_lev = bat.read_level();

  imu.readGyro();
  gyroData = imu.getGyro();

  imu.readAcc();
  accData = imu.getAcc();
  float az = accData.z;

  float pitchm = atan2(accData.z, accData.y);
  float rollm = atan2(accData.z, accData.x);

  pitch = pitch + gyroData.x * dt;
  P_pitch = P_pitch + dt * dt * std_pitch * std_pitch;
  float K_pitch = P_pitch / (P_pitch + dt * dt * 9.0f);

  pitch = pitch + K_pitch * (pitchm - pitch);
  P_pitch = (1 - K_pitch) * P_pitch;

  roll = roll + gyroData.y * dt;
  P_roll = P_roll + dt * dt * std_roll * std_roll;
  float K_roll = P_roll / (P_roll + dt * dt * 9.0f);

  roll = roll + K_roll * (rollm - roll);
  P_roll = (1 - K_roll) * P_roll;

  float pitchd = pitch * 180.0f / 3.14159f;
  float rolld = roll * 180.0f / 3.14159f;

  B = {0.5f * dt * dt, dt};
  F = {1.0f, dt, 0.0f, 1.0f};
  Q = {0.25f * dt * dt * dt * dt * sigma_acc * sigma_acc, 0.5f * dt * dt * dt * sigma_acc * sigma_acc, 0.5f * dt * dt * dt * sigma_acc * sigma_acc, dt * dt * sigma_acc * sigma_acc};

  filter.get_measurement(alt);
  filter.get_command(accData.z - 9.67f);
  filter.estimate(F,B,Q);
  filter.update(H,R);
  BLA::Matrix<2,1,float> x = filter.get_state();
  float altK = x(0,0);

  udp.beginPacket(laptop_IP, udp_port);
  udp.write((uint8_t*)&bat_lev, sizeof(bat_lev));
  //udp.write((uint8_t*)&alt, sizeof(alt));
  //udp.write((uint8_t*)&az, sizeof(az));
  //udp.write((uint8_t*)&altK, sizeof(altK));
  udp.write((uint8_t*)&pitchd, sizeof(pitchd));
  udp.write((uint8_t*)&rolld, sizeof(rolld));
  udp.endPacket();

  //Serial.print(alt); Serial.print(" "); Serial.print(altK); Serial.print(" "); Serial.println(bat_lev);
}

