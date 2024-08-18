#include "hotspot.h"
#include <string.h>

httpd_handle_t Hotspot::httpServerInstance = NULL;
httpd_uri_t Hotspot::index = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_handler,
    .user_ctx = NULL
};

Hotspot::Hotspot()
{
    m_ssid = new char[14];
    strcpy(m_ssid, "PlantFlow");
    m_password = new char[14];
    strcpy(m_password, "plantflow123");
    m_channel = 1;
    m_max_connection = 4;
}

Hotspot::~Hotspot()
{
    delete[] m_ssid;
    delete[] m_password;
}

void Hotspot::startServer()
{
    wifiStart();
}

/***************
******WIFI******
***************/

void Hotspot::wifiStart()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = (uint8_t)strlen(m_ssid),
            .channel = m_channel,
#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
            .authmode = WIFI_AUTH_WPA3_PSK,
            .max_connection = m_max_connection,
            .pmf_cfg = {
                    .required = true,
            },
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
#else /* CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT */
            .authmode = WIFI_AUTH_WPA2_PSK,
            .max_connection = MAX_STA_CONN,
            .pmf_cfg = {
                    .required = true,
            },
#endif
        },
    };
    strcpy((char*)wifi_config.ap.ssid, m_ssid);
    strcpy((char*)wifi_config.ap.password, m_password);

    if (strlen(m_password) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "hotspot init finished. SSID:%s password:%s channel:%d",
             m_ssid, m_password, m_channel);
}

void Hotspot::wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
        startHttpServer();
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d, reason=%d",
                 MAC2STR(event->mac), event->aid, event->reason);
        stopHttpServer();
    }
}

/***************
******HTTP******
***************/

esp_err_t Hotspot::index_handler(httpd_req_t* req)
{
    const char* resp_str = "<html><body><h1>Hello, PlantFlow!</h1></body></html>";
    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
}

void Hotspot::startHttpServer(void)
{
    httpd_config_t httpServerConfiguration = HTTPD_DEFAULT_CONFIG();
    //httpServerConfiguration.server_port = SERVER_PORT;
    if(httpd_start(&httpServerInstance, &httpServerConfiguration) == ESP_OK){
        ESP_ERROR_CHECK(httpd_register_uri_handler(httpServerInstance, &index));
    }
}

void Hotspot::stopHttpServer(void)
{
    if(httpServerInstance != NULL){
        ESP_ERROR_CHECK(httpd_stop(httpServerInstance));
    }
}