#include <stdio.h>
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dht11.h"
#include "waterlevel.h"

extern "C" void app_main(void)
{
    /****************
    ******SETUP******
    ****************/ 

    printf("Hello world!\n");
    DHT11_init(GPIO_NUM_4);
    WaterLevel waterLevel(ADC1_CHANNEL_6);

    while(1) {
        /***************
        ******LOOP******
        ***************/

        auto res = DHT11_read();
        printf("Result: %d\n", res.status);
        printf("Temperature: %d\n", res.temperature);
        printf("Humidity: %d\n", res.humidity);
        auto level = waterLevel.level();
        switch (level)
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
        printf("Voltage: %ld\n", waterLevel.voltage());
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}