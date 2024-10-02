#include "serverconnection.h"

#include <cstring>

#include "cJSON.h"
#include "settings.h"


#define TAG "serverconnection"

static const std::string URL("http://192.168.1.57:8080/plantflow/api/v1/");

ServerConnection::ServerConnection()
{
    m_userId = Settings::instance().userId;
    m_potId = Settings::instance().potId;
    ESP_LOGI(TAG, "ServerConnection created");
    //m_url = "http://192.168.1.57:8080/plantflow/api/v1";
    esp_http_client_config_t config = {
        .url = URL.c_str(),
        .timeout_ms = 50000,
    };
    m_client = esp_http_client_init(&config);
    // test if userId and plantId are valids
}

bool ServerConnection::start()
{
    auto test = [this] -> bool {
        if (!m_userId.empty() && !m_potId.empty()) {
            auto buf = get("pots/" + m_potId);
            if (buf.empty()) {
                m_userId.clear();
                m_potId.clear();
                ESP_LOGI(TAG, "Invalid userId or plantId");
            }
        }
        return !m_userId.empty() && !m_potId.empty();
    };
    if (!test()) {
        ESP_LOGI(TAG, "ServerConnection created");
        m_userId = userId(Settings::instance().username.c_str(), Settings::instance().password.c_str());
        m_potId = plantId();
        if (test()) {
            Settings::instance().userId = m_userId;
            Settings::instance().potId = m_potId;
            Settings::instance().save();
        } else {
            ESP_LOGE(TAG, "Failed to get userId and plantId");
        }
    } else {
        ESP_LOGI(TAG, "userId and plantId okay");
    }
    if (!isValid())
        return false;
     const esp_timer_create_args_t periodic_timer_args = {
        .callback = &ServerConnection::sendSensorData,
        /* name is optional, but may help identify the timer when debugging */
        .arg = this,
        .name = "periodic",
    };
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &m_periodicTimer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(m_periodicTimer, 5000000));
    return true;
}

ServerConnection &ServerConnection::instance()
{
    static ServerConnection instance;
    return instance;
}

std::string ServerConnection::userId(const char *name, const char *password)
{
    if (!m_userId.empty()) {
        return m_userId;
    }
    ESP_LOGI(TAG, "Getting userId...");
    auto body = get("authenticate", name, password);
    cJSON *root = cJSON_Parse(body.c_str());
    if (root == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return "";
    }
    cJSON *userId = cJSON_GetObjectItem(root, "id");
    if (userId == NULL) {
        ESP_LOGE(TAG, "Failed to get userId");
        cJSON_Delete(root);
        return "";
    }
    m_userId = userId->valuestring;
    return m_userId;
}

std::string ServerConnection::plantId()
{
    if (!m_potId.empty()) {
        return m_potId;
    }
    ESP_LOGI(TAG, "Getting plantId...");
    auto buf = post("pots", "{\"name\":\"Pot1\"}");
    cJSON *root = cJSON_Parse(buf.c_str());
    if (root == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return "";
    }
    cJSON *plantId = cJSON_GetObjectItem(root, "id");
    if (plantId == NULL) {
        ESP_LOGE(TAG, "Failed to get plantId");
        cJSON_Delete(root);
        return "";
    }
    m_potId = plantId->valuestring;
    return m_potId;
}

void ServerConnection::sendSensorData(void* self)
{
    ESP_LOGI(TAG, "Sending sensor data...");
    auto server = reinterpret_cast<ServerConnection*>(self);
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "sunlightLevel", 0);
    cJSON_AddNumberToObject(root, "tempLevel", 0);
    cJSON_AddNumberToObject(root, "moistureLevel", 0);
    cJSON_AddNumberToObject(root, "waterLevel", 0);
    char *buf = cJSON_Print(root);
    auto res = server->put("pots", buf);
    ESP_LOGI(TAG, "\n%s\n%s", buf, res.c_str());
    free(buf);
    cJSON_Delete(root);
}

std::string ServerConnection::get(const std::string &url, const std::string &user, const std::string &password)
{
    auto u = URL + url;
    ESP_LOGI(TAG, "GET URL: %s", u.c_str());
    esp_http_client_set_url(m_client, u.c_str());
    if (!user.empty() && !password.empty()) {
        esp_http_client_set_username(m_client, user.c_str());
        esp_http_client_set_password(m_client, password.c_str());
        esp_http_client_set_authtype(m_client, HTTP_AUTH_TYPE_BASIC);
    } else {
        esp_http_client_set_username(m_client, "");
        esp_http_client_set_password(m_client, "");
        esp_http_client_set_authtype(m_client, HTTP_AUTH_TYPE_NONE);
        esp_http_client_set_header(m_client, "Authorization", m_userId.c_str());
    }
    esp_http_client_set_method(m_client, HTTP_METHOD_GET);
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
    std::string data(buf);
    free(buf);
    return data;
}

std::string ServerConnection::post(const std::string &url, const std::string &body)
{
    return write(url, body, HTTP_METHOD_POST);
}

std::string ServerConnection::put(const std::string &url, const std::string &body)
{
    return write(url+"/"+m_potId, body, HTTP_METHOD_PUT);
}

std::string ServerConnection::write(const std::string &url, const std::string &body, esp_http_client_method_t method)
{
    auto u = URL + url;
    ESP_LOGI(TAG, "WRITE URL: %s", u.c_str());
    esp_http_client_set_url(m_client, u.c_str());
    esp_http_client_set_method(m_client, method);
    esp_http_client_set_username(m_client, "");
    esp_http_client_set_password(m_client, "");
    esp_http_client_set_authtype(m_client, HTTP_AUTH_TYPE_NONE);
    esp_http_client_set_header(m_client, "Authorization", m_userId.c_str());
    esp_http_client_open(m_client, body.size());
    esp_http_client_write(m_client, body.c_str(), body.size());
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
    esp_http_client_read(m_client, buf, content_length);
    std::string data(buf);
    free(buf);
    return data;
}