#include "motors.h"
#include "esp_log.h"

static const char* TAG = "motors";
static i2c_master_bus_handle_t bus_handle;

//////////////////////////////////////////////////
// Relevant motor driver register addresses
//////////////////////////////////////////////////
typedef enum : uint8_t
{
    REG_DRV_CONFIG0_ADR =      0x09,    
    REG_DRV_CONFIG3_ADR =      0x0C,
    REG_DRV_CONFIG4_ADR =      0x0D,
    REG_DRV_STATUS1_ADR =      0x04,
    REG_DRV_STATUS2_ADR =      0x05,

} motor_driver_register_address_t;


//////////////////////////////////////////////////
// Register bit positions for motor drivers
//////////////////////////////////////////////////
typedef enum : uint8_t
{
    // CONFIG3 bits
    REG_DRV_CONFIG3_IMODE_POS =     6,
    REG_DRV_CONFIG3_SMODE_POS =     5,    
    REG_DRV_CONFIG3_OCP_MODE_POS =  1,    
    REG_DRV_CONFIG3_TSD_MODE_POS =  0,    

    // CONFIG4 bits 
    REG_DRV_CONFIG4_PMODE_POS =     3,
    REG_DRV_CONFIG4_STALL_REP_POS = 5,

    // CONFIG0 bit
    REG_DRV_CONFIG0_EN_OUT_POS =    7,
    REG_DRV_CONFIG0_EN_OVP_POS =    6,
    REG_DRV_CONFIG0_CLR_FLT_POS =   1,

} motor_driver_register_bit_pos_t;


//////////////////////////////////////////////////
// Active register values for motor drivers
//////////////////////////////////////////////////
typedef enum : uint8_t
{
    REG_DRV_CONFIG3_VAL =      (uint8_t)((0 << REG_DRV_CONFIG3_IMODE_POS) | (0 << REG_DRV_CONFIG3_SMODE_POS) | (1 << REG_DRV_CONFIG3_OCP_MODE_POS) | (1 << REG_DRV_CONFIG3_TSD_MODE_POS)),    
    REG_DRV_CONFIG4_VAL =      (uint8_t)((0 << REG_DRV_CONFIG4_PMODE_POS) | (1 << REG_DRV_CONFIG4_STALL_REP_POS)),
    REG_DRV_CONFIG0_VAL =      (uint8_t)((1 << REG_DRV_CONFIG0_EN_OUT_POS) | (1 << REG_DRV_CONFIG0_EN_OVP_POS)) | (1 << REG_DRV_CONFIG0_CLR_FLT_POS),

} motor_driver_register_values_t;


//////////////////////////////////////////////////
// Packet for writing a register on the drivers
//////////////////////////////////////////////////
typedef struct
{
    motor_driver_register_address_t reg_addr;
    motor_driver_register_values_t reg_val;

} motor_driver_config_packet_t; 


////////////////////////////// Function implementations //////////////////////////////////

void i2c_motor_driver_bus_init()
{

    ESP_LOGI(TAG, "Initializing motor driver i2c bus... ");
    ESP_LOGI(TAG,"Resolution: %d", ledc_find_suitable_duty_resolution(80000000, 32000));

    // initialize I2C bus
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port =             I2C_NUM_1,
        .sda_io_num =           (gpio_num_t) MOTOR_I2C_SDA_PIN,
        .scl_io_num =           (gpio_num_t) MOTOR_I2C_SCL_PIN,
        .clk_source =           I2C_CLK_SRC_XTAL,
        .glitch_ignore_cnt =    7,
        .intr_priority =        0,
        .trans_queue_depth =    0,
        .flags = {
            .enable_internal_pullup = false,
            .allow_pd =               false,
        }
    };
    i2c_new_master_bus(&bus_cfg, &bus_handle);

    // Initialize LEDC PWM timer 0 for all motors
    ledc_timer_config_t timer_cfg = {
        .speed_mode =           LEDC_LOW_SPEED_MODE,
        .duty_resolution =      LEDC_TIMER_11_BIT,
        .timer_num =            LEDC_TIMER_0,
        .freq_hz =              32000,
        .clk_cfg =              LEDC_USE_APB_CLK,
        .deconfigure =          false,
    };
    ledc_timer_config(&timer_cfg);
}

ledc_channel_t get_channel_for_pin(drone_pin_t pwm_pin)
{
    switch (pwm_pin)
    {
    case MOTOR_1_PIN:
        return LEDC_CHANNEL_1;

    case MOTOR_2_PIN:
        return LEDC_CHANNEL_2;

    case MOTOR_3_PIN:
        return LEDC_CHANNEL_3;

    case MOTOR_4_PIN:
        return LEDC_CHANNEL_4;

    default:
        return LEDC_CHANNEL_1;
    }    
}

uint32_t get_hpoint_for_pin(drone_pin_t pwm_pin)
{
    switch (pwm_pin)
    {
        case MOTOR_1_PIN:
            return 0;

        case MOTOR_2_PIN:
            return 512;

        case MOTOR_3_PIN:
            return 1024;

        case MOTOR_4_PIN:
            return 1536;

        default:
            return 0;
    }    
}

void Motor::init(motor_driver_i2c_address_t motor_i2c_address, drone_pin_t pin)
{    
    address = motor_i2c_address;
    pwm_pin = pin;
    channel = get_channel_for_pin(pwm_pin);
    hpoint = get_hpoint_for_pin(pwm_pin);

    // initialize I2C slave device for motor
    slave_cfg = {
        .dev_addr_length =      I2C_ADDR_BIT_LEN_7,
        .device_address =       (uint8_t) motor_i2c_address,
        .scl_speed_hz =         400000, // DRV8215 supports i2c fast mode only [400k max]
        .scl_wait_us =          0,
        .flags = {
           .disable_ack_check = 0,
        },
    };
    
    // Write configuration values to motor registers
    motor_driver_config_packet_t config_packet;
    i2c_master_bus_add_device(bus_handle, &slave_cfg, &handle);
    
    config_packet.reg_addr = REG_DRV_CONFIG3_ADR;
    config_packet.reg_val = REG_DRV_CONFIG3_VAL;
    i2c_master_transmit(handle, (uint8_t*)&config_packet, sizeof(config_packet), -1);

    config_packet.reg_addr = REG_DRV_CONFIG4_ADR;
    config_packet.reg_val = REG_DRV_CONFIG4_VAL;
    i2c_master_transmit(handle, (uint8_t*)&config_packet, sizeof(config_packet), -1);

    config_packet.reg_addr = REG_DRV_CONFIG0_ADR;
    config_packet.reg_val = REG_DRV_CONFIG0_VAL;
    i2c_master_transmit(handle, (uint8_t*)&config_packet, sizeof(config_packet), -1);

    // Initialize LEDC channel for motor
    ledc_channel_config_t channel_cfg = {
        .gpio_num =         (int) pwm_pin,
        .speed_mode =       LEDC_LOW_SPEED_MODE,
        .channel =          channel,
        .intr_type =        LEDC_INTR_DISABLE, // deprecated?
        .timer_sel =        LEDC_TIMER_0,
        .duty =             0,
        .hpoint =           0,
        .sleep_mode =       LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
        .flags = {
           .output_invert = false,
        },
        .deconfigure =      false,
    };
    ledc_channel_config(&channel_cfg);

}   

void Motor::set_duty(float duty_cycle)
{
    uint32_t duty_cycle_int = (uint32_t)(duty_cycle * 2048.0f);

    ESP_LOGI(TAG, "Duty Cycle: %f, %d", duty_cycle, duty_cycle_int);

    // guard to restrict duty_cycle to 11 bits
    if(duty_cycle_int >= 2048)
    {
        duty_cycle_int = 2048;
    }
    ledc_set_duty_with_hpoint(LEDC_LOW_SPEED_MODE, channel, duty_cycle_int, hpoint);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
}

float Motor::read_current()
{
    uint32_t current;
    uint8_t addr = REG_DRV_STATUS2_ADR;
    uint8_t val;
    i2c_master_transmit_receive(handle, &addr, sizeof(addr), &val, sizeof(val), -1);

    current = ((float) (val - 1) * (4000.0f/192.0f)); // magic numbers
    return current;
}


float Motor::read_voltage()
{
    uint32_t voltage;
    uint8_t addr = REG_DRV_STATUS1_ADR;
    uint8_t val;
    i2c_master_transmit_receive(handle, &addr, sizeof(addr), &val, sizeof(val), -1);

    voltage = ((float) (val - 3) * (10813.0f/176.0f)); // magic numbers
    return voltage;
}
