#pragma once

#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

enum Brightness {
    DARK,
    DIM,
    BRIGHT
};

class BrightnessLevel {
    public:
        BrightnessLevel(adc1_channel_t channel, gpio_num_t gpio);
        uint32_t voltage();
        Brightness level();
    private:
        void read();
        esp_adc_cal_characteristics_t m_characteristics;
        adc1_channel_t m_channel;
        gpio_num_t m_gpio;
        uint32_t m_voltage;
        bool m_isDark;
};

/**
 * <150: Bright
 * 150-1000: Dim
 * >1000: Dark
**/