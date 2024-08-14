#pragma once

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

enum Moisture {
    DRY,
    MOIST,
    WET
};

class SoilMoisture {
    public:
        SoilMoisture(adc1_channel_t channel);
        uint32_t voltage();
        Moisture level();
    private:
        void read();
        esp_adc_cal_characteristics_t m_characteristics;
        adc1_channel_t m_channel;
        uint32_t m_voltage;
};