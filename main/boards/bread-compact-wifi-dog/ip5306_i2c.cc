#include "ip5306_i2c.h"
#include "board.h"
#include "display.h"

#include <esp_log.h>

#define TAG "IP5306"

IP5306::IP5306(i2c_master_bus_handle_t i2c_bus, uint8_t addr) : I2cDevice(i2c_bus, addr) {
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
    switch (ReadReg(0x77) & 0b11110000)
    {
    case 0x00:
        return 100;
    case 0x80:
        return 75;
    case 0xC0:
        return 50;
    case 0xE0:
        return 25;
    default:
        return 0;
    }
}

void IP5306::PowerOff() {
    WriteReg(0x01, ReadReg(0x01)|0b00100000);
    WriteReg(0x00, ReadReg(0x00)&0b11011111);
}
