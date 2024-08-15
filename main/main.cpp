#include <stdio.h>
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "brightnesslevel.h"
#include "dht11.h"
#include "hotspot.h"
#include "soilmoisture.h"
#include "waterlevel.h"
#include "waterpump.h"

extern "C" void app_main(void)
{
    /****************
    ******SETUP******
    ****************/ 
    ///PINS
    adc1_channel_t brightnessLevel_channel = ADC1_CHANNEL_4;
    gpio_num_t brightnessLevel_gpio = GPIO_NUM_27;
    gpio_num_t dht11_gpio = GPIO_NUM_4;
    adc1_channel_t soilMoisture_channel = ADC1_CHANNEL_5;
    adc1_channel_t waterLevel_channel = ADC1_CHANNEL_6;
    gpio_num_t waterPump_gpio = GPIO_NUM_19;

    ///HOTSPOT
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "STARTING HOTSPOT");
    Hotspot hotspot;

    ///SENSORS
    ESP_LOGI("sensors", "STARTING SENSORS");
    BrightnessLevel brightnessLevel(brightnessLevel_channel, brightnessLevel_gpio);
    DHT11_init(dht11_gpio);
    SoilMoisture soilMoisture(soilMoisture_channel);
    WaterLevel waterLevel(waterLevel_channel);
    WaterPump waterPump(waterPump_gpio);

    while(1) {
        /***************
        ******LOOP******
        ***************/
     
        auto res = DHT11_read();
        printf("Result: %d\n", res.status);
        printf("Temperature: %d\n", res.temperature);
        printf("Humidity: %d\n", res.humidity);

        auto water_lvl = waterLevel.level();
        switch (water_lvl)
        {
        case EMPTY:
            printf("Water level: Empty\n");
            break;
        case LOW:
            printf("Water level: Low\n");
            break;
        case MEDIUM:
            printf("Water level: Medium\n");
            break;
        case FULL:
            printf("Water level: Full\n");
            break;
        default:
            break;
        }
        printf("Water level voltage: %ld\n", waterLevel.voltage());

        auto brightness_lvl = brightnessLevel.level();
        switch (brightness_lvl)
        {
        case BRIGHT:
            printf("Brightness level: Bright\n");
            break;
        case DIM:
            printf("Brightness level: Dim\n");
            break;
        case DARK:
            printf("Brightness level: Dark\n");
            break;
        default:
            break;
        }
        printf("Brightness voltage: %ld\n", brightnessLevel.voltage());
        auto soil_moisture = soilMoisture.level();
        switch (soil_moisture)
        {
        case DRY:
            printf("Soil moisture: Dry\n");
            break;
        case MOIST:
            printf("Soil moisture: Moist\n");
            break;
        case WET:
            printf("Soil moisture: Wet\n");
            break;
        default:
            break;
        }
        printf("Soil moisture voltage: %ld\n", soilMoisture.voltage());
#if 0
        waterPump.on();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        waterPump.off();
#endif
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}