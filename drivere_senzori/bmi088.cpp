#include "bmi088.h"

BMI088::BMI088(TwoWire* wire){
  acc.x = 0; acc.y = 0; acc.z = 0;
  gyro.x = 0; gyro.y = 0; gyro.z = 0;

  this->wire = wire;
}

void BMI088::setMeas(){
 ////////////////////// ACC
  wire->beginTransmission(ACC_ADR);
  wire->write(ACC_PWR_CTRL);
  wire->write(0x04);//powermode=normal
  wire->write(ACC_CONF);
  wire->write(0xA8);//config low pass filter 
  wire->write(ACC_RANGE);
  wire->write(0x01);//range=6g
  wire->endTransmission();

 ////////////////////// GYRO
  // Power up the gyro
  wire->beginTransmission(GYRO_ADR);
  wire->write(GYRO_PWR_CTRL);
  wire->write(0x00); // normal mode
  wire->write(GYRO_RANGE);
  wire->write(0x00); // ±2000 dps
  wire->endTransmission();
  wire->write(GYRO_BW);
  wire->write(0x07); // 100 Hz ODR, 32 Hz bandwidth as example
  wire->endTransmission();
}

void BMI088::readAcc() {
  wire->beginTransmission(ACC_ADR);
  wire->write(ACC_X_LSB);
  wire->endTransmission();
  wire->requestFrom(ACC_ADR, 0x06);

  byte x_lsb = wire->read();
  byte x_msb = wire->read();
  byte y_lsb = wire->read();
  byte y_msb = wire->read();
  byte z_lsb = wire->read();
  byte z_msb = wire->read();

  int16_t raw_x = (x_msb << 8) | x_lsb;
  int16_t raw_y = (y_msb << 8) | y_lsb;
  int16_t raw_z = (z_msb << 8) | z_lsb;

  acc.x = (float) raw_x * 0.001796f;
  acc.y = (float) raw_y * 0.001796f;
  acc.z = (float) raw_z * 0.001796f;
}

void BMI088::readGyro() {
  wire->beginTransmission(GYRO_ADR);
  wire->write(GYRO_X_LSB);
  wire->endTransmission();
  wire->requestFrom(GYRO_ADR, 0x06);

  byte x_lsb = wire->read();
  byte x_msb = wire->read();
  byte y_lsb = Wire.read();
  byte y_msb = wire->read();
  byte z_lsb = wire->read();
  byte z_msb = wire->read();

  int16_t raw_x = (x_msb << 8) | x_lsb;
  int16_t raw_y = (y_msb << 8) | y_lsb;
  int16_t raw_z = (z_msb << 8) | z_lsb;

  gyro.x = (float) (raw_x * 0.061f);
  gyro.y = (float) (raw_y * 0.061f);
  gyro.z = (float) (raw_z * 0.061f);
}

vector BMI088::getAcc(){
  return acc;
}

vector BMI088::getGyro(){
  return gyro;
}





