#pragma once

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

class WifiMode {
public:
    enum State {
        DISCONNECTED,
        CONNECTED,
        CONNECTING
    };
    static WifiMode &instance();
    State state() const;
    void connect();
private:
    WifiMode();
    static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,void *event_data);
private:
    State m_state = DISCONNECTED;
};