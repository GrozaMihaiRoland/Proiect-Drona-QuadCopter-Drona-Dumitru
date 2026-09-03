#include <Wire.h>
#include <Arduino.h>

#define ACC_ADR 0x18
#define GYRO_ADR 0x69
#define ACC_PWR_CTRL 0x7D;
#define ACC_X_LSB 0x12;
#define ACC_Y_LSB 0x14;
#define ACC_Z_LSB 0x16;
#define ACC_CONF 0x40;
#define ACC_RANGE 0x41;
#define GYRO_PWR_CTRL 0x11;
#define GYRO_RANGE 0x0F;
#define GYRO_BW 0x10;
#define GYRO_X_LSB 0x02;
#define GYRO_Y_LSB 0x04;
#define GYRO_Z_LSB 0x06;

typedef struct vec3{
  float x = 0;
  float y = 0;
  float z = 0;
} vector;

class BMI088{
  private:
    vector acc;
    vector gyro;
    TwoWire* wire;

  public:
    BMI088(TwoWire *wire);
    void setMeas();
    void readGyro();
    void readAcc();
    vector getAcc();
    vector getGyro();
};