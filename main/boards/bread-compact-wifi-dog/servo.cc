#include "iot/thing.h"
#include "board.h"

#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_log.h>

#include "boards/bread-compact-wifi-dog/config.h"

#define TAG "Servo"

namespace iot {

// 这里仅定义 Servo 的属性和方法，不包含具体的实现
class Servo : public Thing {
private:
    int angle_ = SERVO_IDLE_ANGLE;

    typedef struct {
        gpio_num_t gpio;
        ledc_channel_t channel;
    } servo_config_t;

    servo_config_t servos[4];

    inline long map(long x, long in_min, long in_max, long out_min, long out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    void InitializeServo() {
        // Set the LEDC peripheral configuration
        // Prepare and then apply the LEDC PWM timer configuration
        ledc_timer_config_t ledc_timer = {
            .speed_mode = LEDC_MODE,
            .duty_resolution = LEDC_DUTY_RES,
            .timer_num = LEDC_TIMER,
            .freq_hz = LEDC_FREQUENCY,
            .clk_cfg = LEDC_AUTO_CLK
        };
        ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

        servos[0]={SERVO_0_GPIO,SERVO_0_LEDC_CHANNEL};
        servos[1]={SERVO_1_GPIO,SERVO_1_LEDC_CHANNEL};
        servos[2]={SERVO_2_GPIO,SERVO_2_LEDC_CHANNEL};
        servos[3]={SERVO_3_GPIO,SERVO_3_LEDC_CHANNEL};
        
        for (auto &servo : servos) {
            ledc_channel_config_t ledc_channel = {
                .gpio_num = servo.gpio,
                .speed_mode = LEDC_MODE,
                .channel = servo.channel,
                .timer_sel = LEDC_TIMER,
                // .intr_type = LEDC_INTR_DISABLE,
                .duty = 0, // Set duty to 0%
                .hpoint = 0
            };
            ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
        }
    }

public:
    Servo() : Thing("Servo", "用于测试的舵机"), angle_(SERVO_IDLE_ANGLE) {
        InitializeServo();

        // 定义设备的属性
        properties_.AddNumberProperty("angle", "舵机当前角度", [this]() -> int {
            return angle_;
        });
        
        // 定义设备可以被远程执行的指令
        methods_.AddMethod("SetAngle", "设置舵机角度", ParameterList({
            Parameter("angle", "0到180之间的整数", kValueTypeNumber, true)
        }), [this](const ParameterList& parameters) {
            for (auto &servo : servos) {
                ESP_ERROR_CHECK(
                    ledc_set_duty(
                        LEDC_MODE, servo.channel, map(
                            (static_cast<uint8_t>(parameters["angle"].number())), 
                            SERVO_MIN_ANGLE, 
                            SERVO_MAX_ANGLE, 
                            SERVO_MIN_DUTY, 
                            SERVO_MAX_DUTY
                        )
                    )
                );
                // Update duty to apply the new value
                ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, servo.channel));
            }
        });
    }
};

} // namespace iot

DECLARE_THING(Servo);
