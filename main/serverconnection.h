#pragma once

#include <string>

#include "esp_system.h"
#include "esp_http_client.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

class ServerConnection {
public:
    static ServerConnection &instance();
    std::string userId(const char *name, const char *password);
private:
    ServerConnection();
private:
    esp_http_client_handle_t m_client;
};