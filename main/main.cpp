#include <memory>
#include <stdio.h>

#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "brightnesslevel.h"
#include "dht11.h"
#include "hotspot.h"
#include "settings.h"
#include "sensormanager.h"
#include "soilmoisture.h"
#include "waterlevel.h"
#include "waterpump.h"
#include "wifimode.h"

/// COMMENT OUT TO DISABLE SENSORS
// #define SENSOR_OFF
/// COMMENT OUT TO ENABLE WIFI
// #define PUMP_ONLY
#define NVS_TAG "nvs"

extern "C" void app_main(void)
{
    /****************
    ******SETUP******
    ****************/ 
    ///NVS
#ifndef PUMP_ONLY
    ESP_LOGI(NVS_TAG, "Starting NVS");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
    ///PINS
    // adc1_channel_t brightnessLevel_channel = ADC1_CHANNEL_4;
    // gpio_num_t brightnessLevel_gpio = GPIO_NUM_27;
    // gpio_num_t dht11_gpio = GPIO_NUM_4;
    // adc1_channel_t soilMoisture_channel = ADC1_CHANNEL_5;
    // adc1_channel_t waterLevel_channel = ADC1_CHANNEL_6;
    ESP_LOGI("wifi loop", "STARTING");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();
    WifiMode::instance().connect();
    while (true) {
        if (!Settings::instance().ssid.empty()) {
            // WIFI mode
            //ESP_LOGI("wifi loop", "STARTING WIFI MODE");
            if (WifiMode::instance().state() == WifiMode::CONNECTED) {
                ESP_LOGI("wifi loop", "WIFI MODE STARTED (WHILE LOOP)");
                break;
            } else if (WifiMode::instance().state() == WifiMode::DISCONNECTED) {
                ESP_LOGW("wifi loop", "WIFI MODE FAILED TO START (WHILE LOOP)");
                Settings::instance().ssid.clear();
            } else {
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
        } else {
            // HOTSPOT mode
            {
                ESP_LOGI("wifi loop", "STARTING HOTSPOT MODE");
                Hotspot hotspot;
                hotspot.wifiSetup();
            }
            WifiMode::instance().connect();
        }
    }
    ESP_LOGW("wifi loop", "WIFI MODE ENDED");
    ///SENSORS
#endif // PUMP_ONLY
#ifndef SENSOR_OFF
#ifdef PUMP_ONLY
    ESP_LOGE("pump", "PUMP IT UP");
    gpio_num_t waterPump_gpio = GPIO_NUM_19;
    WaterPump waterPump(waterPump_gpio);
    waterPump.on();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    waterPump.off();
    ESP_LOGE("pump", "PUMP IT DOWN");
#endif // PUMP_ONLY
    while(true) {
#ifndef PUMP_ONLY
        SensorManager::instance().update();
#endif // PUMP_ONLY
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
#endif // SENSOR_OFF
}