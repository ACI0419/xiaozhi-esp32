#include "ip5306_i2c.h"
#include "board.h"
#include "display.h"
#include <esp_adc/adc_oneshot.h>
#include <esp_log.h>

#define TAG "IP5306"

IP5306::IP5306(i2c_master_bus_handle_t i2c_bus, uint8_t addr) : I2cDevice(i2c_bus, addr) {
    // 初始化 ADC 单元
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle_));

    // 配置 ADC 通道
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_0,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle_, ADC_CHANNEL_0, &config));

    example_adc_calibration_init(ADC_UNIT_1, ADC_CHANNEL_0, ADC_ATTEN_DB_0, &adc_cali_handle);
}

IP5306::~IP5306() {
    if (adc_handle_) {
        // 释放 ADC 句柄
        ESP_ERROR_CHECK(adc_oneshot_del_unit(adc_handle_));
    }
}

bool IP5306::IsCharging() {
    return (ReadReg(0x70) & 0b0001000) >> 3;
}

bool IP5306::IsDischarging() {
    return !((ReadReg(0x70) & 0b0001000) >> 3);
}

bool IP5306::IsChargingDone() {
    return (ReadReg(0x71) & 0b0001000) >> 3;
}

int IP5306::GetBatteryLevel() {
    int adc_value, voltage;
    // 读取 ADC 值
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle_, ADC_CHANNEL_0, &adc_value));

    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, adc_value, &voltage));

    voltage*=6;
    // 根据电压计算电量百分比
    // 3000mV 对应 0%，4300mV对应 100%
    float percentage = ((voltage - 3000.0f) / (4300 - 3000)) * 100;
    ESP_LOGW(TAG, "ADC RAW value: %d, Battery voltage: %d mV, Battery level: %.2f", adc_value, voltage, percentage);

    return static_cast<int>(percentage);
}

void IP5306::PowerOff() {
    // WriteReg(0x01, ReadReg(0x01)|0b00100000);
    // WriteReg(0x00, ReadReg(0x00)&0b11011111);
}

bool IP5306::example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

void IP5306::example_adc_calibration_deinit(adc_cali_handle_t handle)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}
