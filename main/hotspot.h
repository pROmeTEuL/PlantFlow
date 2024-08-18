#pragma once

#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define TAG "hotspot"

class Hotspot {
public:
    Hotspot();
    ~Hotspot();
    void startServer();

private:
    void wifiStart();
    static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data);
    static esp_err_t index_handler(httpd_req_t* req);
    static void startHttpServer(void);
    static void stopHttpServer(void);

private:
    char* m_ssid;
    char* m_password;
    uint8_t m_channel;
    uint8_t m_max_connection;
    static httpd_handle_t httpServerInstance;
    static httpd_uri_t index;
};