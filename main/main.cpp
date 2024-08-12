#include <stdio.h>
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dht11.h"

extern "C" void app_main(void)
{
    printf("Hello world!\n");
    DHT11_init(GPIO_NUM_4);
    while(1) {
        auto res = DHT11_read();
        printf("Result: %d\n", res.status);
        printf("Temperature: %d\n", res.temperature);
        printf("Humidity: %d\n", res.humidity);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}