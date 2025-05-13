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
        methods_.AddMethod("SetLie", "设置机器狗趴下", ParameterList(), [this](const ParameterList& parameters) {
            motion_="趴下";
            if(currentTaskHandle == nullptr){
                xTaskCreate(
                    [](void* arg) {
                        Dog* dog = static_cast<Dog*>(arg);

                        dog->servos[0].write(180);
                        dog->servos[2].write(0);
                        dog->servos[1].write(0);
                        dog->servos[3].write(180);

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
        methods_.AddMethod("SetFront", "设置机器狗前进", ParameterList(), [this](const ParameterList& parameters) {
            motion_="前进";
            if(currentTaskHandle == nullptr){
                xTaskCreate(
                    [](void* arg) {
                        Dog* dog = static_cast<Dog*>(arg);

                        dog->servos[1].write(140); //舵机2旋转至140度
                        dog->servos[2].write(40);  //舵机旋转至40度
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        dog->servos[0].write(40);  //内容同上
                        dog->servos[3].write(140);
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        dog->servos[1].write(90);
                        dog->servos[2].write(90);
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        dog->servos[0].write(90);
                        dog->servos[3].write(90);
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        dog->servos[0].write(140);
                        dog->servos[3].write(40);
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        dog->servos[1].write(40);
                        dog->servos[2].write(140);
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        dog->servos[0].write(90);
                        dog->servos[3].write(90);
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        dog->servos[1].write(90);
                        dog->servos[2].write(90);

                        dog->motion_ = "站立";
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
        methods_.AddMethod("SetBack", "设置机器狗后退", ParameterList(), [this](const ParameterList& parameters) {
            motion_="后退";
            if(currentTaskHandle == nullptr){
                xTaskCreate(
                    [](void* arg) {
                        Dog* dog = static_cast<Dog*>(arg);

                        dog->servos[2].write(140);
                        dog->servos[1].write(40);
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        dog->servos[3].write(40);
                        dog->servos[0].write(140);
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        dog->servos[2].write(90);
                        dog->servos[1].write(90);
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        dog->servos[3].write(90);
                        dog->servos[0].write(90);
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        dog->servos[3].write(140);
                        dog->servos[0].write(40);
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        dog->servos[2].write(40);
                        dog->servos[1].write(140);
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        dog->servos[3].write(90);
                        dog->servos[0].write(90);
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        dog->servos[2].write(90);
                        dog->servos[1].write(90);

                        dog->motion_ = "站立";
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
        methods_.AddMethod("SetLeft", "设置机器狗左转", ParameterList(), [this](const ParameterList& parameters) {
            motion_="左转";
            if(currentTaskHandle == nullptr){
                xTaskCreate(
                    [](void* arg) {
                        Dog* dog = static_cast<Dog*>(arg);

                        int num = 0;
                        while (num < 3)
                        {
                            dog->servos[0].write(80);
                            dog->servos[3].write(80);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[2].write(120);
                            dog->servos[1].write(120);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[0].write(40);
                            dog->servos[3].write(40);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[2].write(140);
                            dog->servos[1].write(140);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[2].write(90);
                            dog->servos[1].write(90);
                            dog->servos[0].write(90);
                            dog->servos[3].write(90);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[0].write(100);
                            dog->servos[3].write(100);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[2].write(60);
                            dog->servos[1].write(60);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[0].write(90);
                            dog->servos[3].write(90);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[2].write(40);
                            dog->servos[1].write(40);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[2].write(90);
                            dog->servos[1].write(90);

                            num++;
                        }
                        dog->motion_ = "站立";
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
        methods_.AddMethod("SetRight", "设置机器狗右转", ParameterList(), [this](const ParameterList& parameters) {
            motion_="右转";
            if(currentTaskHandle == nullptr){
                xTaskCreate(
                    [](void* arg) {
                        Dog* dog = static_cast<Dog*>(arg);

                        int num = 0;
                        while (num < 3)//调用一次执行3次
                        {
                            dog->servos[0].write(100);
                            dog->servos[3].write(100);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[2].write(60);
                            dog->servos[1].write(60);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[0].write(140);
                            dog->servos[3].write(140);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[2].write(40);
                            dog->servos[1].write(40);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[2].write(90);
                            dog->servos[1].write(90);
                            dog->servos[0].write(90);
                            dog->servos[3].write(90);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[0].write(80);
                            dog->servos[3].write(80);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[2].write(120);
                            dog->servos[1].write(120);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[0].write(90);
                            dog->servos[3].write(90);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[2].write(140);
                            dog->servos[1].write(140);
                            vTaskDelay(300/portTICK_PERIOD_MS);
                            dog->servos[2].write(90);
                            dog->servos[1].write(90);

                            num++;
                        }
                        dog->motion_ = "站立";
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
        
        servos[0].writeImd(90);
        vTaskDelay(300/portTICK_PERIOD_MS);
        servos[1].writeImd(90);
        vTaskDelay(300/portTICK_PERIOD_MS);
        servos[2].writeImd(90);
        vTaskDelay(300/portTICK_PERIOD_MS);
        servos[3].writeImd(90);
        vTaskDelay(300/portTICK_PERIOD_MS);
    }
};

} // namespace iot

DECLARE_THING(Dog);
