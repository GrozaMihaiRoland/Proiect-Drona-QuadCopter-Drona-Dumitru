#include "battery.h"
#include "esp_log.h"

static const char* TAG = "battery";
adc_oneshot_unit_handle_t adc_handle;
adc_cali_handle_t cali_handle;
adc_channel_t channel;

void battery_init()
{
    // adc unit 1 initialization
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_RTC_CLK_SRC_RC_FAST,
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };
    adc_oneshot_new_unit(&init_config, &adc_handle);

    // channel configuration GPIO 10 = 9
    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12
    };
    adc_oneshot_io_to_channel((gpio_num_t)BATTERY_SENSE_PIN, &init_config.unit_id, &channel);
    adc_oneshot_config_channel(adc_handle, channel, &chan_config);
    ESP_LOGI(TAG, "ADC unit %d initialized with channel %d", init_config.unit_id, channel);

    // adc calibration
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = init_config.unit_id,
        .chan = channel,
        .atten = chan_config.atten,
        .bitwidth = chan_config.bitwidth
    };
    adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle);
}

int battery_read()
{
    int raw, result;
    adc_oneshot_get_calibrated_result(adc_handle, cali_handle, channel, &raw);
    result = raw * VOLTAGE_DIVIDER_RATIO - VOLTAGE_OFFSET; // Adjust for voltage divider
    return result;
}