#pragma once

#include <algorithm>
#include <string>
#include <string.h>

#include "cJSON.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "settings.h"

#define TAG "hotspot"

class Hotspot {
    
public:
    Hotspot();
    ~Hotspot();
    void wifiStart();
private:
    static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data);
    static esp_err_t index_handler(httpd_req_t* req);
    static esp_err_t post_handler(httpd_req_t* req);
    void startHttpServer(void);
    void stopHttpServer(void);

private:
    httpd_handle_t httpServerInstance = nullptr;
    const httpd_uri_t index = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler,
        .user_ctx = nullptr
    };
    const httpd_uri_t post = {
        .uri = "/submit",
        .method = HTTP_POST,
        .handler = post_handler,
        .user_ctx = nullptr
    };
};