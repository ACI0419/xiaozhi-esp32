#ifndef __IP5306_I2C_H__
#define __IP5306_I2C_H__

#include "i2c_device.h"

class IP5306 : public I2cDevice {
public:
    IP5306(i2c_master_bus_handle_t i2c_bus, uint8_t addr);
    bool IsCharging();
    bool IsDischarging();
    bool IsChargingDone();
    int GetBatteryLevel();
    void PowerOff();
};

#endif
