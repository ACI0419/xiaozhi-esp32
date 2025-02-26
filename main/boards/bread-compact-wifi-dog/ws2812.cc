#include "iot/thing.h"
#include "board.h"

#include <esp_log.h>

#include "led/circular_strip.h"
#include "boards/bread-compact-wifi-dog/config.h"

#define TAG "WS2812"

namespace iot {

// 这里仅定义 WS2812 的属性和方法，不包含具体的实现
class WS2812 : public Thing {
private:
    bool power_ = false;

    CircularStrip *ws2812;

    void InitializeWS2812() {
        ws2812 = new CircularStrip(WS2812_GPIO, WS2812_NUM);
    }

public:
    WS2812() : Thing("WS2812", "一个测试用的灯"), power_(false) {
        InitializeWS2812();

        // 定义设备的属性
        properties_.AddBooleanProperty("power", "灯是否打开", [this]() -> bool {
            return power_;
        });

        // 定义设备可以被远程执行的指令
        methods_.AddMethod("TurnOn", "打开灯", ParameterList(), [this](const ParameterList& parameters) {
            power_ = true;
            ws2812->myfunc((DeviceState)1);
        });

        methods_.AddMethod("TurnOff", "关闭灯", ParameterList(), [this](const ParameterList& parameters) {
            power_ = false;
            ws2812->myfunc((DeviceState)3);
        });
    }
};

} // namespace iot

DECLARE_THING(WS2812);
