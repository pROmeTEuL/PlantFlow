#include "serverconnection.h"

#include <cstring>
#include "cJSON.h"

#define TAG "serverconnection"

ServerConnection::ServerConnection() {
    ESP_LOGI(TAG, "ServerConnection created");
    esp_http_client_config_t config = {
        .url = "http://192.168.1.57:8080/plantflow/api/v1/authenticate",
        // .host = "http://129.168.1.148",
        // .port = 8080,
        .username = "radu",
        .password = "parola",
        .auth_type = HTTP_AUTH_TYPE_BASIC,
        // .path = "/plantflow/api/v1/authenticate",
        .method = HTTP_METHOD_GET,
        
    };
    m_client = esp_http_client_init(&config);
}

ServerConnection &ServerConnection::instance() {
    static ServerConnection instance;
    return instance;
}

std::string ServerConnection::userId(const char *name, const char *password) {
    ESP_LOGI(TAG, "Getting userId...");
    // esp_http_client_set_username(m_client, name);
    // esp_http_client_set_password(m_client, password);
    auto err = esp_http_client_open(m_client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open connection");
        return "";
    }
    ESP_LOGI(TAG, "Connection opened; READING DATA...");

    // Get http body
    int content_length = esp_http_client_fetch_headers(m_client);
    int status_code = esp_http_client_get_status_code(m_client);
    ESP_LOGI(TAG, "Status code: %d", status_code);
    if (status_code != 200) {
        ESP_LOGE(TAG, "Failed to get status code");
        return "";
    }
    ESP_LOGI(TAG, "Content length: %d", content_length);
    char *buf = (char*)malloc(content_length + 1);
    memset(buf, 0, content_length + 1);
    err = esp_http_client_read(m_client, buf, content_length);
    if (err < 0) {
        ESP_LOGE(TAG, "Failed to read data");
        free(buf);
        return "";
    }
    ESP_LOGI(TAG, "Data read: %s", buf);
    std::string result(buf);
    free(buf);
#warining TODO: parse JSON
    return result;
}
