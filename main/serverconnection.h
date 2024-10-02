#pragma once

#include <string>

#include "esp_system.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

class ServerConnection {
    ServerConnection();
public:
    static ServerConnection &instance();
    bool start();
    bool isValid() const { return !m_userId.empty() && !m_potId.empty(); }

    static void sendSensorData(void* self);

private:
    std::string userId(const char *name, const char *password);
    std::string plantId();
    std::string get(const std::string &url, const std::string &user = "", const std::string &password = "");
    std::string post(const std::string &url, const std::string &body);
    std::string put(const std::string &url, const std::string &body);
    std::string write(const std::string &url, const std::string &body, esp_http_client_method_t method);

private:
    esp_http_client_handle_t m_client;
    std::string m_userId;
    std::string m_potId;
    std::string m_name = "Pot1";
    esp_timer_handle_t m_periodicTimer;
};