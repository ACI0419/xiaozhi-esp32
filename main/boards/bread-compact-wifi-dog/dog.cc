#include "iot/thing.h"
#include "board.h"
#include "servo.h"

#include <string>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "boards/bread-compact-wifi-dog/config.h"

#define TAG "Dog"

namespace iot {

// 这里仅定义 Dog 的属性和方法，不包含具体的实现
class Dog : public Thing {
private:
    std::string motion_ = DOG_DEFAULT_MOTION;
    bool running_ = false;

    TaskHandle_t currentTaskHandle = nullptr;

    Servo servos[4]={
        Servo(SERVO_0_GPIO,LEDC_CHANNEL_4),    //CHANNEL_0被lcd背光占用
        Servo(SERVO_1_GPIO,LEDC_CHANNEL_5),
        Servo(SERVO_2_GPIO,LEDC_CHANNEL_6),
        Servo(SERVO_3_GPIO,LEDC_CHANNEL_7)
    };

public:
    Dog() : Thing("Dog", "一只可以控制的机器狗"), motion_(DOG_DEFAULT_MOTION) {
        // 定义设备的属性
        properties_.AddStringProperty("Dog", "机器狗当前动作", [this]() -> std::string {
            return motion_;
        });
        
        // 定义设备可以被远程执行的指令
        methods_.AddMethod("SetSit", "设置机器狗坐下", ParameterList(), [this](const ParameterList &parameters){
            motion_="坐下";
            if(currentTaskHandle == nullptr){
                xTaskCreate(
                    [](void* arg) {
                        Dog* dog = static_cast<Dog*>(arg);

                        dog->servos[0].write(90);
                        dog->servos[1].write(140);
                        dog->servos[2].write(90);
                        dog->servos[3].write(40);

                        dog->currentTaskHandle = nullptr;
                        vTaskDelete(nullptr);
                    },
                    "SitTask",
                    2048,
                    this,
                    5,
                    &currentTaskHandle
                );
            }
        });
        methods_.AddMethod("SetStand", "设置机器狗站立", ParameterList(), [this](const ParameterList& parameters) {
            motion_="站立";
            if(currentTaskHandle == nullptr){
                xTaskCreate(
                    [](void* arg) {
                        Dog* dog = static_cast<Dog*>(arg);

                        dog->servos[0].write(90);
                        dog->servos[1].write(90);
                        dog->servos[2].write(90);
                        dog->servos[3].write(90);

                        dog->currentTaskHandle = nullptr;
                        vTaskDelete(nullptr);
                    },
                    "StandTask",
                    2048,
                    this,
                    5,
                    &currentTaskHandle
                );
            }
        });
        servos[0].write(90);
        servos[1].write(90);
        servos[2].write(90);
        servos[3].write(90);
    }
};

} // namespace iot

DECLARE_THING(Dog);
