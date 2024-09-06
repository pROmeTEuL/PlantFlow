#include "settings.h"

#include <stdio.h>
#include <inttypes.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#define NVS_TAG "nvs"

Settings::Settings()
{
    // Open
    ESP_LOGI(NVS_TAG, "Opening Non-Volatile Storage (NVS) handle ... ");
    // Handle will automatically close when going out of scope or when it's reset.
    esp_err_t err = ESP_OK;
    auto handle = nvs::open_nvs_handle("storage", NVS_READONLY, &err);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        printf("Done\n");
        // Read
        printf("Reading restart counter from NVS ... ");
        auto get_string = [&](const char *key, std::string &value) {
            char temp[100];
            esp_err_t err = handle->get_string(key, temp, sizeof(temp));
            if (err == ESP_OK) {
                value = temp;
            }
            return err;
        };
        if (get_string("ssid", ssid) == ESP_OK) {
            get_string("wifi_password", wifi_password);
            get_string("username", username);
            get_string("password", password);
            printf("Found:\nssid: %s\nwifi_password: %s\nusername: %s\npassword: %s\n", ssid.c_str(), wifi_password.c_str(), username.c_str(), password.c_str());
        } else {
            printf("No settings found\n");
        }
    }
}

Settings &Settings::instance()
{
    static Settings instance;
    return instance;
}

void Settings::save()
{
    esp_err_t err = ESP_OK;
    auto handle = nvs::open_nvs_handle("storage", NVS_READWRITE, &err);
    err = handle->set_string("ssid", ssid.c_str());
    if (err != ESP_OK) {
        ESP_LOGE(NVS_TAG, "Error (%s) setting ssid in NVS!", esp_err_to_name(err));
    }
    err = handle->set_string("wifi_password", wifi_password.c_str());
    if (err != ESP_OK) {
        ESP_LOGE(NVS_TAG, "Error (%s) setting wifi_password in NVS!", esp_err_to_name(err));
    }
    err = handle->set_string("username", username.c_str());
    if (err != ESP_OK) {
        ESP_LOGE(NVS_TAG, "Error (%s) setting username in NVS!", esp_err_to_name(err));
    }
    err = handle->set_string("password", password.c_str());
    if (err != ESP_OK) {
        ESP_LOGE(NVS_TAG, "Error (%s) setting password in NVS!", esp_err_to_name(err));
    }
    err = handle->commit();
    if (err != ESP_OK) {
        ESP_LOGE(NVS_TAG, "Error (%s) committing updates in NVS!", esp_err_to_name(err));
    }
    //handle->close();
}