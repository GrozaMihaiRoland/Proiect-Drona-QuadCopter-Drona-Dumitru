////////////////////////// Includes ///////////////////////////////////

#include <driver/i2c_master.h>
#include "pins.h"

#pragma once

////////////// Structures and Defines ///////////////////////

#define SET_HEIGHT_ORIGIN_STEPS         256
#define HEIGHT_LIN_APROXIMATION_SLOPE   -11.9018878375f // slope between 90m - 100m above sealevel

/////////////////////////////////////////////////////////////
// 3D floating point vector for Accelerometer and Gyro
/////////////////////////////////////////////////////////////
typedef struct
{
    float x;
    float y;
    float z;

} vector3d_t;


/////////////////////////////////////////////////////////////
// Floating point trimming coeficients used by the barometer
/////////////////////////////////////////////////////////////
typedef struct
{
    float    T1;
    float    T2;
    float    T3;
    float    P1;
    float    P2;
    float    P3;
    float    P4;
    float    P5;
    float    P6;
    float    P7;
    float    P8;
    float    P9;
    float    P10;
    float    P11;  

} barometer_trim_coeff_float_t;


/////////////// Function Declarations /////////////////////////////////

void i2c_sensor_bus_init();

/////////////////// Class Declarations ////////////////////////////////

class Sensor
{
    protected:
    i2c_master_dev_handle_t handle;
    i2c_device_config_t slave_cfg;

    public:
    void init();
    void read(){};
};

class Accelerometer : public Sensor
{
    private:
    vector3d_t accel;
    
    public:
    void init();
    void read();
    vector3d_t get_acceleration();
};

class Gyroscope : public Sensor
{
    private:
    vector3d_t gyro;

    public:
    void init();
    void read();
    vector3d_t get_angular_velocity();
};

class Barometer : public Sensor
{
    private:
    float temperature;
    float pressure;
    float pressure_at_origin;
    float height;

    barometer_trim_coeff_float_t trim;
    
    public:
    void init();
    void read();
    float get_pressure();
    void set_height_origin();
    float get_height();
};
