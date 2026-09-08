#include "sensors.h"
#include "esp_log.h"
#include <freertos/FreeRTOS.h>

/////////////////////// Enums and local variables /////////////////////////////////

static const char* TAG = "sensors";
static i2c_master_bus_handle_t bus_handle;

//////////////////////////////////////////////////
// Sensor I2C addresses (raw)
//////////////////////////////////////////////////
typedef enum : uint8_t
{
    ACCELEROMETER_I2C_ADDRESS = 0x18,
    GYROSCOPE_I2C_ADDRESS = 0x69,
    BAROMETER_I2C_ADDRESS = 0x77,

} sensor_i2c_address_t;


//////////////////////////////////////////////////
// Relevant sensor register addresses
//////////////////////////////////////////////////
typedef enum : uint8_t
{
    // Addresses for accelerometer registers
    REG_ACC_PWR_CTRL_ADR =      0x7D,
    REG_ACC_CONF_ADR =          0x40,
    REG_ACC_RANGE_ADR =         0x41,
    REG_ACC_X_LSB_ADR =         0x12,
    REG_ACC_Y_LSB_ADR =         0x14,
    REG_ACC_Z_LSB_ADR =         0x16,

    // Addresses for gyroscope registers
    REG_GYRO_PWR_CTRL_ADR =     0x11,
    REG_GYRO_RANGE_ADR =        0x0F,
    REG_GYRO_BW_ADR =           0x10,
    REG_GYRO_X_LSB_ADR =        0x02,
    REG_GYRO_Y_LSB_ADR =        0x04,
    REG_GYRO_Z_LSB_ADR =        0x06,

    // Addresses for barometer registers
    REG_BARO_PRES_LSB_ADR =     0x04,
    REG_BARO_TRIM_COEFF_ADR =   0x31,
    REG_BARO_PWR_CTRL_ADR =     0x1B,
    REG_BARO_OSR_ADR =          0x1C,
    REG_BARO_IIR_CONFIG_ADR =   0x1F,
    REG_BARO_ODR_ADR =          0x1D,

} sensor_register_address_t;


//////////////////////////////////////////////////
// Active register values for sensors
//////////////////////////////////////////////////
typedef enum : uint8_t
{
    // Values for accelerometer registers
    REG_ACC_PWR_CTRL_VAL =      0x04, // powermode = normal
    REG_ACC_CONF_VAL =          0xA8, // lowpass configuration
    REG_ACC_RANGE_VAL =         0x01, // range = 6g
    
    // Values for gyroscope registers
    REG_GYRO_PWR_CTRL_VAL =     0x00, // powermode = normal
    REG_GYRO_RANGE_VAL =        0x00, // range = +/-2000dps
    REG_GYRO_BW_VAL =           0x07, // bandwidth = 100 Hz ODR, 32 Hz bandwidth as an example

    // Values for barometer registers
    REG_BARO_PWR_CTRL_VAL =     0x33, // mode = normal + t_en(set) + p_en(set)
    REG_BARO_OSR_VAL =          0x03, // osr_p = x8, osr_t = x1
    REG_BARO_IIR_CONFIG_VAL =   0x04, // filter coeff = 3 
    REG_BARO_ODR_VAL =          0x02, // ODR = 50 Hz

} sensor_register_values_t;


//////////////////////////////////////////////////
// Packet for writing a register on the sensors
//////////////////////////////////////////////////
typedef struct
{
    sensor_register_address_t reg_addr;
    sensor_register_values_t reg_val;

} sensor_config_packet_t; 


//////////////////////////////////////////////////
// Packet for receiving x,y,z data from sensors
//////////////////////////////////////////////////
typedef struct
{
    int16_t x_raw;
    int16_t y_raw;
    int16_t z_raw;

} sensor_data_packet_t;


//////////////////////////////////////////////////
// Raw trimming coefficients for the barometer
//////////////////////////////////////////////////
typedef struct __attribute__((packed))
{
    uint16_t    T1;
    uint16_t    T2;
    int8_t      T3;
    int16_t     P1;
    int16_t     P2;
    int8_t      P3;
    int8_t      P4;
    uint16_t    P5;
    uint16_t    P6;
    int8_t      P7;
    int8_t      P8;
    int16_t     P9;
    int8_t      P10;
    int8_t      P11;  

} barometer_trim_coeff_packet_t;

///////////////////// Function Definitions /////////////////////////////////

void i2c_sensor_bus_init()
{
    ESP_LOGI(TAG, "Initializing sensor bus... ");
    static_assert(sizeof(barometer_trim_coeff_packet_t) == 21, "BMP388 trim struct must be exactly 21 bytes");

    // initialize I2C bus
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = (gpio_num_t)SENSOR_I2C_SDA_PIN,
        .scl_io_num = (gpio_num_t)SENSOR_I2C_SCL_PIN,
        .clk_source = I2C_CLK_SRC_XTAL,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {
            .enable_internal_pullup = false,
            .allow_pd = false
        }
    };
    i2c_new_master_bus(&bus_cfg, &bus_handle);
}

void Accelerometer::init()
{
    // initialize I2C slave device for accelerometer
    slave_cfg = {
        .dev_addr_length =          I2C_ADDR_BIT_LEN_7,
        .device_address =           ACCELEROMETER_I2C_ADDRESS,
        .scl_speed_hz =             1000000,
        .scl_wait_us =              0,
        .flags = {
            .disable_ack_check =    0
        }
    };

    // Write configuration values to accelerometer registers
    i2c_master_bus_add_device(bus_handle, &slave_cfg, &handle);

    sensor_config_packet_t config_packet;
    config_packet.reg_addr = REG_ACC_PWR_CTRL_ADR;
    config_packet.reg_val = REG_ACC_PWR_CTRL_VAL;
    i2c_master_transmit(handle, (uint8_t*)&config_packet, sizeof(config_packet), -1);

    config_packet.reg_addr = REG_ACC_CONF_ADR;
    config_packet.reg_val = REG_ACC_CONF_VAL;
    i2c_master_transmit(handle, (uint8_t*)&config_packet, sizeof(config_packet), -1);

    config_packet.reg_addr = REG_ACC_RANGE_ADR;
    config_packet.reg_val = REG_ACC_RANGE_VAL;
    i2c_master_transmit(handle, (uint8_t*)&config_packet, sizeof(config_packet), -1);
}

void Gyroscope::init()
{
    slave_cfg = {
        .dev_addr_length =          I2C_ADDR_BIT_LEN_7,
        .device_address =           GYROSCOPE_I2C_ADDRESS,
        .scl_speed_hz =             1000000,
        .scl_wait_us =              0,
        .flags = {
            .disable_ack_check =    0
        }
    };

    
    // Write configuration values to gyroscope registers
    sensor_config_packet_t config_packet;
    i2c_master_bus_add_device(bus_handle, &slave_cfg, &handle);

    config_packet.reg_addr = REG_GYRO_PWR_CTRL_ADR;
    config_packet.reg_val = REG_GYRO_PWR_CTRL_VAL;
    i2c_master_transmit(handle, (uint8_t*)&config_packet, sizeof(config_packet), -1);

    config_packet.reg_addr = REG_GYRO_RANGE_ADR;
    config_packet.reg_val = REG_GYRO_RANGE_VAL;
    i2c_master_transmit(handle, (uint8_t*)&config_packet, sizeof(config_packet), -1);

    config_packet.reg_addr = REG_GYRO_BW_ADR;
    config_packet.reg_val = REG_GYRO_BW_VAL;
    i2c_master_transmit(handle, (uint8_t*)&config_packet, sizeof(config_packet), -1);
}

void Barometer::init()
{
    ESP_LOGI(TAG, "Initializing barometer... ");
    slave_cfg = {
        .dev_addr_length =          I2C_ADDR_BIT_LEN_7,
        .device_address =           BAROMETER_I2C_ADDRESS,
        .scl_speed_hz =             1000000,
        .scl_wait_us =              0,
        .flags = {
            .disable_ack_check =    0
        }
    };

    sensor_config_packet_t config_packet;
    i2c_master_bus_add_device(bus_handle, &slave_cfg, &handle);

    config_packet.reg_addr = REG_BARO_PWR_CTRL_ADR;
    config_packet.reg_val = REG_BARO_PWR_CTRL_VAL;
    i2c_master_transmit(handle, (uint8_t*)&config_packet, sizeof(config_packet), -1);

    config_packet.reg_addr = REG_BARO_OSR_ADR;
    config_packet.reg_val = REG_BARO_OSR_VAL;
    i2c_master_transmit(handle, (uint8_t*)&config_packet, sizeof(config_packet), -1);

    config_packet.reg_addr = REG_BARO_IIR_CONFIG_ADR;
    config_packet.reg_val = REG_BARO_IIR_CONFIG_VAL;
    i2c_master_transmit(handle, (uint8_t*)&config_packet, sizeof(config_packet), -1);

    config_packet.reg_addr = REG_BARO_ODR_ADR;
    config_packet.reg_val = REG_BARO_ODR_VAL;
    i2c_master_transmit(handle, (uint8_t*)&config_packet, sizeof(config_packet), -1);
    
    // Trim coefficient read and calculation
    sensor_register_address_t nvm_trim_coeff_address = REG_BARO_TRIM_COEFF_ADR;
    barometer_trim_coeff_packet_t trim_packet;
    i2c_master_transmit_receive(
        handle,
        (uint8_t*)&nvm_trim_coeff_address,
        sizeof(nvm_trim_coeff_address),
        (uint8_t*)&trim_packet,
        sizeof(trim_packet),
        -1
    );

    ESP_LOGI(
        TAG,
        "Coefficients read %u, %u, %d, %d, %d, %d, %d, %u, %u, %d, %d, %d, %d, %d",
        trim_packet.T1,
        trim_packet.T2,
        trim_packet.T3,
        trim_packet.P1,
        trim_packet.P2,
        trim_packet.P3,
        trim_packet.P4,
        trim_packet.P5,
        trim_packet.P6,
        trim_packet.P7,
        trim_packet.P8,
        trim_packet.P9,
        trim_packet.P10,
        trim_packet.P11
    );

    // convert calibration coeficient with the formulas in the datasheet
    trim.T1 = ((float) trim_packet.T1) / 0.00390625f;               // T1 / 2^-8
    trim.T2 = ((float) trim_packet.T2) / 1073741824.0f;             // T2 / 2^30
    trim.T3 = ((float) trim_packet.T3) / 281474976710656.0f;        // T3 / 2^48
    trim.P1 = ((float) (trim_packet.P1 - 16384)) / 1048576.0f;      // (P1 - 2^14) / 2^20
    trim.P2 = ((float) (trim_packet.P2 - 16384)) / 536870912.0f;    // (P2 - 2^14) / 2^29
    trim.P3 = ((float) trim_packet.P3) / 4294967296.0f;             // P3 / 2^32
    trim.P4 = ((float) trim_packet.P4) / 137438953472.0f;           // P4 / 2^37
    trim.P5 = ((float) trim_packet.P5) / 0.125f;                    // P5 / 2^-3
    trim.P6 = ((float) trim_packet.P6) / 64.0f;                     // P6 / 2^6
    trim.P7 = ((float) trim_packet.P7) / 256.0f;                    // P7 / 2^8
    trim.P8 = ((float) trim_packet.P8) / 32768.0f;                  // P8 / 2^15
    trim.P9 = ((float) trim_packet.P9) / 281474976710656.0f;        // P9 / 2^48
    trim.P10 = ((float) trim_packet.P10) / 281474976710656.0f;      // P10 / 2^48
    trim.P11 = ((float) trim_packet.P11) / 36893488147419103232.0f; // P11 / 2^65 


}

void Accelerometer::read()
{
    // Read accelerometer data from registers
    sensor_register_address_t packet_addr = REG_ACC_X_LSB_ADR;
    sensor_data_packet_t packet;
    i2c_master_transmit_receive(
        handle, 
        (uint8_t*)&packet_addr, 
        sizeof(packet_addr), 
        (uint8_t*)&packet, 
        sizeof(packet), 
        -1
    );
    
    accel.x = (float)(packet.x_raw * 0.01796f);
    accel.y = (float)(packet.y_raw * 0.01796f);
    accel.z = (float)(packet.z_raw * 0.01796f);

    ESP_LOGI(TAG, "Accel data: %f %f %f", accel.x, accel.y, accel.z);
}

void Gyroscope::read()
{
    // Read gyroscope data from registers
    sensor_register_address_t packet_addr = REG_GYRO_X_LSB_ADR;
    sensor_data_packet_t packet;
    i2c_master_transmit_receive(
        handle,
        (uint8_t*)&packet_addr,
        sizeof(packet_addr),
        (uint8_t*)&packet,
        sizeof(packet),
        -1
    );

    gyro.x = (float)(packet.x_raw * 0.061035f);
    gyro.y = (float)(packet.y_raw * 0.061035f);
    gyro.z = (float)(packet.z_raw * 0.061035f);

    ESP_LOGI(TAG, "Gyro data: %f %f %f", gyro.x, gyro.y, gyro.z);
}

void Barometer::read()
{
    // Read barometer data from registers
    sensor_register_address_t packet_addr = REG_BARO_PRES_LSB_ADR;
    uint8_t data[6];
    i2c_master_transmit_receive(
        handle, 
        (uint8_t*)&packet_addr, 
        sizeof(packet_addr), 
        (uint8_t*)&data, 
        sizeof(data), 
        -1
    );
    uint32_t raw_pressure = (data[2] << 16) | (data[1] << 8) | data[0]; 
    uint32_t raw_temperature = (data[5] << 16) | (data[4] << 8) | data[3]; 

    float partial_data1;
    float partial_data2;
    float partial_data3;
    float partial_out1;
    float partial_out2;

    // Temperature compensation
    partial_data1 = (float)(raw_temperature - trim.T1);
    partial_data2 = (float)(partial_data1 * trim.T2);

    temperature = partial_data2 + (partial_data1 * partial_data1) * trim.T3;

    // Pressure compensation
    float raw_pressure_float = (float)raw_pressure;
    float temperature_squared = temperature * temperature;
    float temperature_cubed = temperature * temperature_squared;

    partial_data1 = trim.P6 * temperature;
    partial_data2 = trim.P7 * temperature_squared;
    partial_data3 = trim.P8 * temperature_cubed;
    partial_out1 = trim.P5 + partial_data1 + partial_data2 + partial_data3; 

    partial_data1 = trim.P2 * temperature;
    partial_data2 = trim.P3 * temperature_squared;
    partial_data3 = trim.P4 * temperature_cubed;
    partial_out2 = raw_pressure_float * (trim.P1 + partial_data1 + partial_data2 + partial_data3);

    partial_data1 = raw_pressure_float * raw_pressure_float;
    partial_data2 = trim.P9 + trim.P10 * temperature;
    partial_data3 = partial_data1 * partial_data2 + (raw_pressure_float * raw_pressure_float * raw_pressure_float) * trim.P11;

    pressure = partial_out1 + partial_out2 + partial_data3;
}

vector3d_t Accelerometer::get_acceleration()
{
    return accel;
}

vector3d_t Gyroscope::get_angular_velocity()
{
    return gyro;
}

float Barometer::get_pressure()
{
    return pressure;
}

void Barometer::set_height_origin()
{
    float avg_pressure = 0.0f;
    float avg_scale_down_factor = 1.0f/SET_HEIGHT_ORIGIN_STEPS;
    for(uint32_t i = 0; i < SET_HEIGHT_ORIGIN_STEPS; i++)
    {
        this->read();
        avg_pressure = avg_pressure + avg_scale_down_factor * pressure;
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG, "Average pressure: %f Pa", avg_pressure);
    pressure_at_origin = avg_pressure;
}

float Barometer::get_height()
{
    this->read();
    height = (pressure - pressure_at_origin) / HEIGHT_LIN_APROXIMATION_SLOPE; 
    ESP_LOGI(TAG, "Height %f mm, Pressure: %f Pa, Temperature %f C", height*1000.0f, pressure, temperature);
    return height;
}
