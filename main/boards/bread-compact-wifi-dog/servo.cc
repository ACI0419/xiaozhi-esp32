#include <esp_log.h>

#include "servo.h"
#include "boards/bread-compact-wifi-dog/config.h"

#define TAG "Servo"

Servo::Servo(gpio_num_t gpio,ledc_channel_t channel){
    this->gpio_ = gpio;
    this->channel_ = channel;

    ledc_channel_config_t ledc_channel = {
        .gpio_num = gpio,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = channel,
        .timer_sel = LEDC_TIMER_3,
        // .intr_type = LEDC_INTR_DISABLE,
        .duty = 0, // Set duty to 0%
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

long Servo::map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Servo::write(int angle){
    long target_duty = map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, SERVO_MIN_DUTY, SERVO_MAX_DUTY);
    // 设置渐变时间，单位为毫秒，可以根据需求调整
    const int fade_time_ms = 200; 

    // 设置渐变目标占空比和时间
    ESP_ERROR_CHECK(
        ledc_set_fade_with_time(
            LEDC_LOW_SPEED_MODE,
            channel_,
            target_duty,
            fade_time_ms
        )
    );
    // 启动渐变
    ESP_ERROR_CHECK(
        ledc_fade_start(
            LEDC_LOW_SPEED_MODE,
            channel_,
            LEDC_FADE_NO_WAIT
        )
    );
}
