#include "wifimode.h"
#include <string.h>
#include "settings.h"
#include "serverconnection.h"

#define WIFI_TAG "wifi"

WifiMode::WifiMode()
{
    ESP_LOGI(WIFI_TAG, "STARTING WIFI MODE");
    esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, this));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, this));
}

WifiMode &WifiMode::instance()
{
    static WifiMode instance;
    return instance;
}

WifiMode::State WifiMode::state() const
{
    return m_state;
}

void WifiMode::connect()
{
    if (m_state != DISCONNECTED) {
        return;
    }
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));
    strcpy((char*)wifi_config.sta.ssid, Settings::instance().ssid.c_str());
    strcpy((char*)wifi_config.sta.password, Settings::instance().wifi_password.c_str());
    wifi_config.sta.failure_retry_cnt = 2;
    wifi_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
    printf("WIFI SSID: %s\n", wifi_config.sta.ssid);
    printf("WIFI Password: %s\n", wifi_config.sta.password);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_OK) {
        ESP_LOGI(WIFI_TAG, "WIFI MODE STARTED (FUNCTION)");
        m_state = CONNECTING;
    } else {
        ESP_LOGE(WIFI_TAG, "WIFI MODE FAILED TO START (FUNCTION)");
        m_state = DISCONNECTED;
    }
}

void WifiMode::wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,void *event_data){
    auto self = reinterpret_cast<WifiMode*>(arg);
    if(event_id == WIFI_EVENT_STA_START) {
        printf("WIFI CONNECTING....\n");
        self->m_state = CONNECTING;
    } else if (event_id == WIFI_EVENT_STA_CONNECTED) {
        printf("WiFi CONNECTED\n");
        self->m_state = CONNECTED;
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        printf("WiFi DISCONNECTED\n");
        self->m_state = DISCONNECTED;
    } else if (event_id == IP_EVENT_STA_GOT_IP) {
        printf("Wifi got IP...\n\n");
        ESP_LOGI("server", "GETTING USERID");
        if (!ServerConnection::instance().start())
        {
            ESP_LOGE("server", "Failed to start server connection");
            self->m_state = DISCONNECTED;
        }
    }
}