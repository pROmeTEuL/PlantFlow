#pragma once

#include <string>
#include <memory>

#include "brightnesslevel.h"
#include "dht11.h"
#include "soilmoisture.h"
#include "waterlevel.h"
#include "waterpump.h"

#include "driver/gpio.h"
#include "driver/adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class SensorManager {
    SensorManager();
public:
    static SensorManager &instance();
    void update();
    void updateMoistureValues(int min, int max);
    std::string sensorsData() const;

private:
    std::unique_ptr<BrightnessLevel> brightnessLevel;
    std::unique_ptr<SoilMoisture> soilMoisture;
    std::unique_ptr<WaterLevel> waterLevel;
    std::unique_ptr<WaterPump> waterPump;

private:
    int minMoisture = -1;
    int maxMoisture = -1;
    int currentMoisture = -1;
    int currentBrightness = -1;
    int currentTemperature = -1;
    int currentWaterLevel = -1;

    const adc1_channel_t m_brightnessLevel_channel = ADC1_CHANNEL_4;
    const gpio_num_t m_brightnessLevel_gpio = GPIO_NUM_27;
    const gpio_num_t m_dht11_gpio = GPIO_NUM_4;
    const adc1_channel_t m_soilMoisture_channel = ADC1_CHANNEL_5;
    const adc1_channel_t m_waterLevel_channel = ADC1_CHANNEL_6;
    const gpio_num_t m_waterPump_gpio = GPIO_NUM_19;
};