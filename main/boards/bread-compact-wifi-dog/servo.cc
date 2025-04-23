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
    ESP_ERROR_CHECK(
        ledc_set_duty(
            LEDC_LOW_SPEED_MODE,
            channel_,
            map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, SERVO_MIN_DUTY, SERVO_MAX_DUTY)
        )
    );
    // Update duty to apply the new value
    ESP_ERROR_CHECK(
        ledc_update_duty(
            LEDC_LOW_SPEED_MODE,
            channel_
        )
    );
}
