#include "sensormanager.h"

#include "cJSON.h"

SensorManager::SensorManager()
{
}

SensorManager &SensorManager::instance()
{
    static SensorManager instance;
    return instance;
}

void SensorManager::fetch()
{
    // fetch data from sensors
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