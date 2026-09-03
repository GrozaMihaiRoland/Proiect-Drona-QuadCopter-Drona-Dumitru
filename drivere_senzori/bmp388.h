#include <Arduino.h>
#include <Wire.h>
#include <math.h>

class BMP388 {
  private:
    uint8_t i2c_addr;
    uint8_t pxlsb;
    uint8_t txlsb;
    uint8_t nvm;

    uint32_t raw_pressure;
    uint32_t raw_temperature;

    float pressure;
    float temperature;
    float altitude;

    uint16_t par_t1;
    uint16_t par_t2;
    int8_t par_t3;
    int16_t par_p1;
    int16_t par_p2;
    int8_t par_p3;
    int8_t par_p4;
    uint16_t par_p5;
    uint16_t par_p6;
    int8_t par_p7;
    int8_t par_p8;
    int16_t par_p9;
    int8_t par_p10;
    int8_t par_p11;

    float pt[3];
    float pp[11];
  
  public: 
    BMP388(uint8_t addr);
    uint32_t read_raw_pressure();
    uint32_t read_raw_temperature();
    void return_comp_data();
    void set_meas();
    float get_pres();
    float get_temp();
    float get_alt(float p0);
};