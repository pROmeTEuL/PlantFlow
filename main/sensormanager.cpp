#include "sensormanager.h"

#include <algorithm>
#include "cJSON.h"

SensorManager::SensorManager()
{
    DHT11_init(m_dht11_gpio);
    brightnessLevel = std::make_unique<BrightnessLevel>(m_brightnessLevel_channel, m_brightnessLevel_gpio);
    soilMoisture = std::make_unique<SoilMoisture>(m_soilMoisture_channel);
    waterLevel = std::make_unique<WaterLevel>(m_waterLevel_channel);
    waterPump = std::make_unique<WaterPump>(m_waterPump_gpio);
}

SensorManager &SensorManager::instance()
{
    static SensorManager instance;
    return instance;
}

void SensorManager::update()
{
    return;
    auto res = DHT11_read();
    printf("Result: %d\n", res.status);
    printf("Temperature: %d\n", res.temperature);
    printf("Humidity: %d\n", res.humidity);
    currentTemperature = res.temperature;
        
    // float temp = 1.f - float(soilMoisture->voltage() / 2000); /// max voltage is above 2000
    currentMoisture = 100 - std::clamp<uint32_t>(soilMoisture->voltage(), 0, 2000) / 20;
    // float temp = 1.f - float(brightnessLevel->voltage() / 1000); /// max voltage is above 1000
    currentBrightness = 100 - std::clamp<uint32_t>(brightnessLevel->voltage(), 0, 1000) / 10;
    // float temp = float(float(waterLevel->voltage() - 1000) / 600); /// max voltage is above 1600 and min is bellow 1000
    currentWaterLevel = (std::clamp<uint32_t>(waterLevel->voltage(), 1000, 1600) - 1000) / 6;
    if (currentMoisture < minMoisture && waterLevel->level() != EMPTY)
    {
        // Commented out for safety measures
        //waterPump->on();
        
    }
    else if (currentMoisture >= maxMoisture || waterLevel->level() == EMPTY)
    {
        // Commented out for safety measures
        //waterPump->off();
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void SensorManager::updateMoistureValues(int min, int max)
{
    minMoisture = min;
    maxMoisture = max;
}

std::string SensorManager::sensorsData() const
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "sunlightLevel", currentBrightness);
    cJSON_AddNumberToObject(root, "tempLevel", currentTemperature);
    cJSON_AddNumberToObject(root, "moistureLevel", currentMoisture);
    cJSON_AddNumberToObject(root, "waterLevel", currentWaterLevel);
    char *buf = cJSON_Print(root);
    std::string res(buf);
    free(buf);
    cJSON_Delete(root);
    return res;
}