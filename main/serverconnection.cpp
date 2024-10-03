#include "serverconnection.h"

#include <cstring>

#include "cJSON.h"
#include "settings.h"
#include "sensormanager.h"


#define TAG "serverconnection"

static const std::string URL("http://192.168.1.57:8080/plantflow/api/v1/");

class HttpClient {
public:
    HttpClient() {
        esp_http_client_config_t config = {
            .url = URL.c_str(),
            .timeout_ms = 10000,
        };
        m_client = esp_http_client_init(&config);
    }
    ~HttpClient() {
        esp_http_client_close(m_client);
        esp_http_client_cleanup(m_client);
    }
    esp_http_client_handle_t get() { return m_client; }
private:
    esp_http_client_handle_t m_client;
};

ServerConnection::ServerConnection()
{
    m_userId = Settings::instance().userId;
    m_potId = Settings::instance().potId;
    ESP_LOGI(TAG, "ServerConnection created");
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
        ESP_LOGE(TAG, "Old userid: %s", m_userId.c_str());
        m_userId = userId(Settings::instance().username.c_str(), Settings::instance().password.c_str());
        ESP_LOGE(TAG, "New userid: %s", m_userId.c_str());
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
    ESP_ERROR_CHECK(esp_timer_start_periodic(m_periodicTimer, 15000000));
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
    static bool running = false;
    if (running) {
        return;
    }
    running = true;
    ESP_LOGI(TAG, "Sending sensor data...");
    auto server = reinterpret_cast<ServerConnection*>(self);
    auto res = server->put("pots", SensorManager::instance().sensorsData());
    cJSON *root = cJSON_Parse(res.c_str());
    if (root) {
        SensorManager::instance().updateMoistureValues(
            cJSON_GetObjectItem(root, "minMoistureLevel")->valueint,
            cJSON_GetObjectItem(root, "maxMoistureLevel")->valueint
        );
    }
    cJSON_Delete(root);
    running = false;
}

std::string ServerConnection::get(const std::string &url, const std::string &user, const std::string &password)
{
    HttpClient client;
    auto u = URL + url;
    ESP_LOGI(TAG, "GET URL: %s", u.c_str());
    esp_http_client_set_url(client.get(), u.c_str());
    if (!user.empty() && !password.empty()) {
        ESP_LOGI(TAG, "Setting username and password");
        esp_http_client_set_username(client.get(), user.c_str());
        esp_http_client_set_password(client.get(), password.c_str());
        esp_http_client_set_authtype(client.get(), HTTP_AUTH_TYPE_BASIC);
    } else {
        ESP_LOGI(TAG, "Authorization: %s", m_userId.c_str());
        esp_http_client_set_username(client.get(), "");
        esp_http_client_set_password(client.get(), "");
        esp_http_client_set_authtype(client.get(), HTTP_AUTH_TYPE_NONE);
        esp_http_client_set_header(client.get(), "Authorization", m_userId.c_str());
    }
    esp_http_client_set_method(client.get(), HTTP_METHOD_GET);
    auto err = esp_http_client_open(client.get(), 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open connection");
        return "";
    }
    ESP_LOGI(TAG, "Connection opened; READING DATA...");

    // Get http body
    int content_length = esp_http_client_fetch_headers(client.get());
    int status_code = esp_http_client_get_status_code(client.get());
    ESP_LOGI(TAG, "Status code: %d", status_code);
    ESP_LOGI(TAG, "Content length: %d", content_length);
    if (status_code == -1 || content_length < 0) {
        ESP_LOGE(TAG, "Failed to write data");
        return "";
    }
    std::string data;
    char *buf = (char*)malloc(1024);
    while(content_length) {
        auto len = esp_http_client_read(client.get(), buf, content_length);
        if (len < 0) {
            ESP_LOGE(TAG, "Failed to read data");
            free(buf);
            return "";
        }
        data.append(buf, len);
        content_length -= len;
    }
    free(buf);

    ESP_LOGI(TAG, "Data: %i\n%s", data.size(), data.c_str());
    if (status_code >= 400) {
        ESP_LOGW(TAG, "DATA CLEAR");
        data.clear();
    }
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

std::string ServerConnection::write(const std::string &url, std::string body, esp_http_client_method_t method)
{
    HttpClient client;
    auto u = URL + url;
    ESP_LOGI(TAG, "WRITE URL: %s", u.c_str());
    ESP_ERROR_CHECK(esp_http_client_set_url(client.get(), u.c_str()));
    ESP_ERROR_CHECK(esp_http_client_set_method(client.get(), method));
    ESP_ERROR_CHECK(esp_http_client_set_username(client.get(), ""));
    ESP_ERROR_CHECK(esp_http_client_set_password(client.get(), ""));
    ESP_ERROR_CHECK(esp_http_client_set_authtype(client.get(), HTTP_AUTH_TYPE_NONE));
    ESP_ERROR_CHECK(esp_http_client_set_header(client.get(), "Authorization", m_userId.c_str()));
    ESP_ERROR_CHECK(esp_http_client_open(client.get(), body.size()));
    while(body.size()) {
        auto len = esp_http_client_write(client.get(), body.c_str(), body.size());
        if (len)
        body = body.substr(len);
    }
    int content_length = esp_http_client_fetch_headers(client.get());
    int status_code = esp_http_client_get_status_code(client.get());
    ESP_LOGI(TAG, "Status code: %d", status_code);
    ESP_LOGI(TAG, "Content length: %d", content_length);
    if (status_code == -1 || content_length < 0) {
        ESP_LOGE(TAG, "Failed to write data");
        return "";
    }
    char *buf = (char*)malloc(content_length + 1);
    memset(buf, 0, content_length + 1);
    if (content_length > 0)
        esp_http_client_read(client.get(), buf, content_length);
    std::string data(buf);
    free(buf);
    if (status_code >= 400) {
        ESP_LOGW(TAG, "DATA CLEAR");
        data.clear();
    }
    return data;
}