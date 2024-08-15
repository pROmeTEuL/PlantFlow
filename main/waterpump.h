#pragma once

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

enum Status {
    OFF, // 0
    ON // 1
};

class WaterPump
{
public:
    WaterPump(gpio_num_t gpio);
    void on();
    void off();
    Status status();
private:
    gpio_num_t m_gpio;
    Status m_status;  
};