#include <driver/ledc.h>
#include <driver/gpio.h>

class Servo{
private:
    gpio_num_t gpio_;
    ledc_channel_t channel_;
    inline long map(long x, long in_min, long in_max, long out_min, long out_max);
public:
    Servo(gpio_num_t gpio,ledc_channel_t channel);
    void write(int angle);
};