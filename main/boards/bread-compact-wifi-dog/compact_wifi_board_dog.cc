#include "wifi_board.h"
#include "audio_codecs/no_audio_codec.h"
#include "display/lcd_display.h"
#include "system_reset.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "iot/thing_manager.h"
#include "led/single_led.h"
#include "ip5306_i2c.h"
#include "power_save_timer.h"
#include "assets/lang_config.h"
#include "esp_sleep.h"
#include <wifi_station.h>
#include <esp_log.h>
#include <driver/i2c_master.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <driver/spi_common.h>

#define TAG "CompactWifiBoardDog"

LV_FONT_DECLARE(font_puhui_16_4);
LV_FONT_DECLARE(font_awesome_16_4);

class Pmic : public IP5306 {
public:
    Pmic(i2c_master_bus_handle_t i2c_bus, uint8_t addr) : IP5306(i2c_bus, addr)
    {
        WriteReg(0x01, ReadReg(0x01)&0b11111011);
        WriteReg(0x21, ReadReg(0x21)&0b00111111);
        WriteReg(0x21, ReadReg(0x21)|0b00011100);
    }
};

class CompactWifiBoardDog : public WifiBoard
{
private:
    LcdDisplay* display_;
    Button boot_button_;
    Button touch_button_;
    Button volume_up_button_;
    Button volume_down_button_;
    Button shutdown_button_;
    i2c_master_bus_handle_t i2c_bus_;
    Pmic* pmic_ = nullptr;
    PowerSaveTimer* power_save_timer_;

    void InitializePowerSaveTimer() {
        power_save_timer_ = new PowerSaveTimer(-1, -1, 600);
        power_save_timer_->OnShutdownRequest([this]() {
            pmic_->PowerOff();
        });
        power_save_timer_->SetEnabled(true);
    }

    void InitializeI2c() {
        i2c_master_bus_config_t bus_config = {
            .i2c_port = (i2c_port_t)0,
            .sda_io_num = SDA_PIN,
            .scl_io_num = SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus_));
    }

    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = DISPLAY_MOSI_PIN;
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = DISPLAY_CLK_PIN;
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    void InitializeLcdDisplay() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;
        // 液晶屏控制IO初始化
        ESP_LOGD(TAG, "Install panel IO");
        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = DISPLAY_CS_PIN;
        io_config.dc_gpio_num = DISPLAY_DC_PIN;
        io_config.spi_mode = DISPLAY_SPI_MODE;
        io_config.pclk_hz = 40 * 1000 * 1000;
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &panel_io));

        // 初始化液晶屏驱动芯片
        ESP_LOGD(TAG, "Install LCD driver");
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_RST_PIN;
        panel_config.rgb_ele_order = DISPLAY_RGB_ORDER;
        panel_config.bits_per_pixel = 16;

        ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io, &panel_config, &panel));

        esp_lcd_panel_reset(panel);

        esp_lcd_panel_init(panel);
        esp_lcd_panel_invert_color(panel, DISPLAY_INVERT_COLOR);
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
        display_ = new SpiLcdDisplay(panel_io, panel,
                                     DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY,
                                     {
                                         .text_font = &font_puhui_16_4,
                                         .icon_font = &font_awesome_16_4,
                                         .emoji_font = DISPLAY_HEIGHT >= 240 ? font_emoji_64_init() : font_emoji_32_init(),
                                     });
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            power_save_timer_->WakeUp();
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }
            app.ToggleChatState();
        });
        touch_button_.OnPressDown([this]() {
            power_save_timer_->WakeUp();
            Application::GetInstance().StartListening();
        });
        touch_button_.OnPressUp([this]() {
            power_save_timer_->WakeUp();
            Application::GetInstance().StopListening();
        });

        volume_up_button_.OnClick([this]() {
            power_save_timer_->WakeUp();
            auto codec = GetAudioCodec();
            auto volume = codec->output_volume() + 10;
            if (volume > 100) {
                volume = 100;
            }
            codec->SetOutputVolume(volume);
            GetDisplay()->ShowNotification(Lang::Strings::VOLUME + std::to_string(volume));
        });

        volume_up_button_.OnLongPress([this]() {
            power_save_timer_->WakeUp();
            GetAudioCodec()->SetOutputVolume(100);
            GetDisplay()->ShowNotification(Lang::Strings::MAX_VOLUME);
        });

        volume_down_button_.OnClick([this]() {
            power_save_timer_->WakeUp();
            auto codec = GetAudioCodec();
            auto volume = codec->output_volume() - 10;
            if (volume < 0) {
                volume = 0;
            }
            codec->SetOutputVolume(volume);
            GetDisplay()->ShowNotification(Lang::Strings::VOLUME + std::to_string(volume));
        });

        volume_down_button_.OnLongPress([this]() {
            power_save_timer_->WakeUp();
            GetAudioCodec()->SetOutputVolume(0);
            GetDisplay()->ShowNotification(Lang::Strings::MUTED);
        });

        shutdown_button_.OnDoubleClick([this]() {
            ESP_LOGW(TAG, "Shutdown, entering light sleep");
            // 配置唤醒源
            gpio_wakeup_enable((gpio_num_t)SHUTDOWN_BUTTON_GPIO, GPIO_INTR_LOW_LEVEL);
            esp_sleep_enable_gpio_wakeup();
            while (gpio_get_level(SHUTDOWN_BUTTON_GPIO) == 0) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            // 进入轻睡眠状态
            esp_light_sleep_start();
            esp_restart();
        });
    }

    void InitializeServoController() {
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
    }

    // 物联网初始化，添加对 AI 可见设备
    void InitializeIot() {
        auto& thing_manager = iot::ThingManager::GetInstance();
        thing_manager.AddThing(iot::CreateThing("Speaker"));
        //thing_manager.AddThing(iot::CreateThing("Lamp")); //和舵机引脚冲突
        thing_manager.AddThing(iot::CreateThing("Dog"));
        thing_manager.AddThing(iot::CreateThing("WS2812"));
        if (DISPLAY_BACKLIGHT_PIN != GPIO_NUM_NC) {
            thing_manager.AddThing(iot::CreateThing("Backlight"));
        }
        thing_manager.AddThing(iot::CreateThing("Battery"));
    }

public:
    CompactWifiBoardDog() :
        boot_button_(BOOT_BUTTON_GPIO),
        touch_button_(TOUCH_BUTTON_GPIO),
        volume_up_button_(VOLUME_UP_BUTTON_GPIO),
        volume_down_button_(VOLUME_DOWN_BUTTON_GPIO),
        shutdown_button_(SHUTDOWN_BUTTON_GPIO) {
        InitializeI2c();
        pmic_ = new Pmic(i2c_bus_, IP5306_I2C_ADDR);
        InitializeSpi();
        InitializeLcdDisplay();
        InitializeButtons();
        InitializeServoController();
        InitializeIot();
        InitializePowerSaveTimer();
        if (DISPLAY_BACKLIGHT_PIN != GPIO_NUM_NC) {
            GetBacklight()->RestoreBrightness();
        }
    }

    virtual Led* GetLed() override {
        static SingleLed led(BUILTIN_LED_GPIO);
        return &led;
    }

    virtual AudioCodec* GetAudioCodec() override {
#ifdef AUDIO_I2S_METHOD_SIMPLEX
        static NoAudioCodecSimplex audio_codec(AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
                                               AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK, AUDIO_I2S_SPK_GPIO_DOUT, AUDIO_I2S_MIC_GPIO_SCK, AUDIO_I2S_MIC_GPIO_WS, AUDIO_I2S_MIC_GPIO_DIN);
#else
        static NoAudioCodecDuplex audio_codec(AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
                                              AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN);
#endif
        return &audio_codec;
    }

    virtual Display* GetDisplay() override {
        return display_;
    }

    virtual Backlight* GetBacklight() override {
        if (DISPLAY_BACKLIGHT_PIN != GPIO_NUM_NC) {
            static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
            return &backlight;
        }
        return nullptr;
    }

    virtual bool GetBatteryLevel(int &level, bool& charging) override {
        static bool last_charging = false;
        charging = pmic_->IsCharging();
        if (charging != last_charging) {
            power_save_timer_->WakeUp();
            last_charging = charging;
        }

        level = pmic_->GetBatteryLevel();

        if (pmic_->IsDischarging()) {
            power_save_timer_->SetEnabled(true);
        } else {
            power_save_timer_->SetEnabled(false);
        }
        return true;
    }
};

DECLARE_BOARD(CompactWifiBoardDog);
