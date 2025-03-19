#include "iot/thing.h"
#include "board.h"

#include <string>

#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_log.h>

#include "boards/bread-compact-wifi-dog/config.h"

#define TAG "Dog"

namespace iot {

// 这里仅定义 Dog 的属性和方法，不包含具体的实现
class Dog : public Thing {
private:
    std::string motion_ = DOG_DEFAULT_MOTION;

    typedef struct {
        gpio_num_t gpio;
        ledc_channel_t channel;
    } servo_config_t;

    servo_config_t servos[4];

    inline long map(long x, long in_min, long in_max, long out_min, long out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    inline void setAngle(servo_config_t servo,int angle){
        ESP_ERROR_CHECK(
            ledc_set_duty(
                LEDC_LOW_SPEED_MODE,
                servo.channel,
                map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, SERVO_MIN_DUTY, SERVO_MAX_DUTY)
            )
        );
        // Update duty to apply the new value
        ESP_ERROR_CHECK(
            ledc_update_duty(
                LEDC_LOW_SPEED_MODE,
                servo.channel
            )
        );
    }

    void InitializeDog() {
        // Set the LEDC peripheral configuration
        // Prepare and then apply the LEDC PWM timer configuration
        ledc_timer_config_t ledc_timer = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = LEDC_TIMER_12_BIT,
            .timer_num = LEDC_TIMER_3,  //TIMER_0被lcd背光占用
            .freq_hz = 50,
            .clk_cfg = LEDC_AUTO_CLK
        };
        ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

        servos[0]={SERVO_0_GPIO,LEDC_CHANNEL_4};    //CHANNEL_0被lcd背光占用
        servos[1]={SERVO_1_GPIO,LEDC_CHANNEL_5};
        servos[2]={SERVO_2_GPIO,LEDC_CHANNEL_6};
        servos[3]={SERVO_3_GPIO,LEDC_CHANNEL_7};
        
        for (auto &servo : servos) {
            ledc_channel_config_t ledc_channel = {
                .gpio_num = servo.gpio,
                .speed_mode = LEDC_LOW_SPEED_MODE,
                .channel = servo.channel,
                .timer_sel = LEDC_TIMER_3,
                // .intr_type = LEDC_INTR_DISABLE,
                .duty = 0, // Set duty to 0%
                .hpoint = 0
            };
            ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
        }
    }

public:
    Dog() : Thing("Dog", "一只可以控制的机器狗"), motion_(DOG_DEFAULT_MOTION) {
        InitializeDog();

        // 定义设备的属性
        properties_.AddStringProperty("Dog", "机器狗当前动作", [this]() -> std::string {
            return motion_;
        });
        
        // 定义设备可以被远程执行的指令
        methods_.AddMethod("SetSit", "设置机器狗坐下", ParameterList(), [this](const ParameterList &parameters){
            motion_="坐";
            setAngle(servos[0],150);
            setAngle(servos[1],150);
            setAngle(servos[2],60);
            setAngle(servos[3],60);
        });
        methods_.AddMethod("SetStand", "设置机器狗站立", ParameterList(), [this](const ParameterList& parameters) {
            motion_="站";
            setAngle(servos[0],90);
            setAngle(servos[1],90);
            setAngle(servos[2],90);
            setAngle(servos[3],90);
        });
    }
};

} // namespace iot

DECLARE_THING(Dog);
