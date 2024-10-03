#pragma once

#include <string>

class SensorManager {
    SensorManager();
public:
    static SensorManager &instance();
    void fetch();
    void updateMoistureValues(int min, int max);
    std::string sensorsData() const;
private:
    int minMoisture = -1;
    int maxMoisture = -1;
    int currentMoisture = -1;
    int currentBrightness = -1;
    int currentTemperature = -1;
    int currentWaterLevel = -1;
};