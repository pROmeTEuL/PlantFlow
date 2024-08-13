#pragma once

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

enum Level {
    EMPTY,
    LOW,
    MEDIUM,
    FULL
};

class WaterLevel {
    public:
        WaterLevel(adc1_channel_t channel);
        uint32_t voltage();
        Level level();
    private:
        void read();
        esp_adc_cal_characteristics_t m_characteristics;
        adc1_channel_t m_channel;
        uint32_t m_voltage;
};

/** 
* <1000: Empty
* 1000-1400: Low
* 1400-1600: Medium
* >1600: Full
**/