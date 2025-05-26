#ifndef __IP5306_I2C_H__
#define __IP5306_I2C_H__

#include "i2c_device.h"
#include <esp_adc/adc_oneshot.h>

class IP5306 : public I2cDevice {
public:
    IP5306(i2c_master_bus_handle_t i2c_bus, uint8_t addr);
    ~IP5306();
    bool IsCharging();
    bool IsDischarging();
    bool IsChargingDone();
    int GetBatteryLevel();
    void PowerOff();
private:
    adc_oneshot_unit_handle_t adc_handle_ = NULL;
    adc_cali_handle_t adc_cali_handle = NULL;
    bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
    void example_adc_calibration_deinit(adc_cali_handle_t handle);
};

#endif
