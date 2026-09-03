#include "bmp388.h"


BMP388::BMP388(uint8_t addr) {
  i2c_addr = addr;
  pxlsb = 0x04;
  txlsb = 0x07;
  nvm = 0x31;

  raw_pressure = 0;
  raw_temperature = 0;

  pressure = 0;
  temperature = 0;

  par_t1 = 0; par_t2 = 0;
  par_t3 = 0; par_p1 = 0;
  par_p2 = 0; par_p3 = 0;
  par_p4 = 0; par_p5 = 0;
  par_p6 = 0; par_p7 = 0;
  par_p8 = 0; par_p9 = 0;
  par_p10 = 0; par_p11 = 0;

  pt[0] = 0; pt[1] = 0;
  pt[2] = 0; pp[0] = 0;
  pp[1] = 0; pp[2] = 0;
  pp[3] = 0; pp[4] = 0;
  pp[5] = 0; pp[6] = 0;
  pp[7] = 0; pp[8] = 0;
  pp[9] = 0; pp[10] = 0;
}

void BMP388::set_meas(){
  /*
  make the following settings:
    mode = normal - 1Bh 00110011
    oss = standard res - ?
    osr_p = x8 - 1Ch 00000011
    osr_t = x1 - 1Ch 00000011
    iir_coef = 2 - 1F 00000100
    idd = 570 uA 
    odr = 50 hz - 1D 0x02
    rms_noise = 11 cm
  */

  Wire.beginTransmission(i2c_addr);
  Wire.write(0x1B);
  Wire.write(0x33); // mode = normal + t_en=1 + p_en=1

  Wire.write(0x1C);
  Wire.write(0x03); // osr_p = x8, osr_t = x1

  Wire.write(0x1F);
  Wire.write(0x04); // filter coeff = 3

  Wire.write(0x1D);
  Wire.write(0x02); // odr = 50 hz
  Wire.endTransmission();
}

uint32_t BMP388::read_raw_pressure() {
  Wire.beginTransmission(i2c_addr);
  Wire.write(pxlsb);
  Wire.endTransmission();

  uint8_t nr = 0x03;
  Wire.requestFrom(i2c_addr, nr);
  byte xlsb = Wire.read();
  byte lsb = Wire.read();
  byte msb = Wire.read();

  raw_pressure = (uint32_t) (msb<<16 | lsb<<8 | xlsb);
  return raw_pressure;
}

uint32_t BMP388::read_raw_temperature() {
  Wire.beginTransmission(i2c_addr);
  Wire.write(txlsb);
  Wire.endTransmission();
  
  uint8_t nr = 0x03;
  Wire.requestFrom(i2c_addr, nr);
  byte xlsb = Wire.read();
  byte lsb = Wire.read();
  byte msb = Wire.read();

  raw_temperature = (uint32_t) (msb<<16 | lsb<<8 | xlsb);
  return raw_temperature;
}

void BMP388::return_comp_data(){
  Wire.beginTransmission(i2c_addr);
  Wire.write(nvm);
  Wire.endTransmission();

  uint8_t data[21];
  uint8_t nr = 0x15;

  Wire.requestFrom(i2c_addr, nr);
  for (int i=0;i<21;i++){
    data[i] = Wire.read();
  }

  par_t1 = (uint16_t) (data[1] << 8) | data[0];
  par_t2 = (uint16_t) (data[3] << 8) | data[2];
  par_t3 = (int8_t) data[4];
  par_p1 = (int16_t) (data[6] << 8) | data[5];
  par_p2 = (int16_t)(data[8] << 8) | data[7];
  par_p3 = (int8_t) data[9];
  par_p4 = (int8_t) data[10];
  par_p5 = (uint16_t) (data[12] << 8) | data[11];
  par_p6 = (uint16_t) (data[14] << 8) | data[13];
  par_p7 = (int8_t) data[15];
  par_p8 = (int8_t) data[16];
  par_p9 = (int16_t)(data[18] << 8) | data[17];
  par_p10 = (int8_t) data[19];
  par_p11 = (int8_t) data[20];

  pt[0] = (float) (par_t1/pow(2,-8));
  pt[1] = (float) (par_t2/pow(2,30));
  pt[2] = (float) (par_t3/pow(2,48));

  pp[0] = (float) ((par_p1-pow(2,14))/pow(2,20));
  pp[1] = (float) ((par_p2-pow(2,14))/pow(2,29));
  pp[2] = (float) (par_p3/pow(2,32));
  pp[3] = (float) (par_p4/pow(2,37));
  pp[4] = (float) (par_p5/pow(2,-3));
  pp[5] = (float) (par_p6/pow(2,6));
  pp[6] = (float) (par_p7/pow(2,8));
  pp[7] = (float) (par_p8/pow(2,15));
  pp[8] = (float) (par_p9/pow(2,48));
  pp[9] = (float) (par_p10/pow(2,48));
  pp[10] = (float) (par_p11/pow(2,65));

  //temp comp

  float pd1 = (float) (raw_temperature - pt[0]);
  float pd2 = (float) (pd1 * pt[1]);

  temperature = pd2 + (pd1 * pd1) * pt[2];

  //pres comp
  pd1 = pp[5] * temperature;
  pd2 = pp[6] * temperature * temperature;
  float pd3 = pp[7] * temperature * temperature * temperature;
  float out1 = pp[4] + pd1 + pd2 + pd3;

  pd1 = pp[1] * temperature;
  pd2 = pp[2] * temperature * temperature;
  pd3 = pp[3] * temperature * temperature * temperature;
  float out2 = (float) (raw_pressure * (pp[0] + pd1 + pd2 + pd3));
  
  pd1 = ((float) raw_pressure) * ((float) raw_pressure);
  pd2 = pp[8] * pp[9] * temperature;
  pd3 = pd1 * pd2;
  float pd4 = pd3 + ((float) raw_pressure) * ((float) raw_pressure) * ((float) raw_pressure) * pp[10];
  pressure = out1 + out2 + pd4; 
}

float BMP388::get_pres(){
  return pressure;
}

float BMP388::get_temp(){
  return temperature;
}

float BMP388::get_alt(float p0){
  return 44330 * (1-pow(get_pres()/p0, 0.1903f));
}