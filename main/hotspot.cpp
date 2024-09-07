#include "hotspot.h"

Hotspot::Hotspot()
{
    ESP_LOGI(TAG, "STARTING HOTSPOT");
}

Hotspot::~Hotspot()
{
    // ESP_LOGI(TAG, "STOPPING ALL CONNECTIONS");
    ESP_LOGI(TAG, "STOPPING HOTSPOT");
    stopHttpServer();
    // ESP_LOGI(TAG, "WIFI DISCONNECT");
    // ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler));
    ESP_LOGI(TAG, "WIFI STOP");
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_LOGI(TAG, "WIFI DEINIT");
    ESP_ERROR_CHECK(esp_wifi_deinit());
}

/***************
******WIFI******
***************/

const std::string_view m_ssid = "PlantFlow";
const std::string_view m_password = "plantflow123";
const uint8_t m_channel = 1;
const uint8_t m_max_connection = 4;

void Hotspot::wifiSetup()
{
    ESP_LOGI(TAG, "NETIF INIT");
    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_LOGI(TAG, "EVENT LOOP CREATE");
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_LOGI(TAG, "NETIF CREATE DEFAULT WIFI AP");

    ESP_LOGI(TAG, "WIFI INIT");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_LOGI(TAG, "EVENT HANDLER REGISTER");
    printf("This: %x\n", (unsigned int)(void*)this);
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        this));
    ESP_LOGI(TAG, "WIFI CONFIG");
    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = (uint8_t)m_ssid.length(),
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
    strcpy((char*)wifi_config.ap.ssid, m_ssid.data());
    strcpy((char*)wifi_config.ap.password, m_password.data());
    ESP_LOGI(TAG, "strcpy");
    if (m_password.length() == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_LOGI(TAG, "WIFI SET MODE");
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_LOGI(TAG, "WIFI SET CONFIG");
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_LOGI(TAG, "WIFI START");
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "hotspot init finished. SSID:%s password:%s channel:%d",
             m_ssid.data(), m_password.data(), m_channel);
    while (m_waiting) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void Hotspot::wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    printf ("Event data: %x\n", (unsigned int)arg);
    Hotspot* self = reinterpret_cast<Hotspot*>(arg);
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
        ESP_LOGI(TAG, "CONNECTED");
        self->startHttpServer();
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d, reason=%d",
                 MAC2STR(event->mac), event->aid, event->reason);
        ESP_LOGI(TAG, "DISCONNECTED");
        self->stopHttpServer();
    }
}

/***************
******HTTP******
***************/

esp_err_t Hotspot::index_handler(httpd_req_t* req)
{
    ESP_LOGI(TAG, "GET request");
    auto resp_str = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Simple Form</title>
</head>
<body>
    <h2>Login Form</h2>
    <form id="loginForm">
        <label for="username">Username:</label>
        <input type="text" id="username" name="username" required><br><br>
        
        <label for="password">Password:</label>
        <input type="password" id="password" name="password" required><br><br>
        
        <label for="ssid">Wifi ssid:</label>
        <input type="text" id="ssid" name="ssid" required><br><br>

        <label for="wifipassword">Wifi password:</label>
        <input type="password" id="wifipassword" name="wifipassword" required><br><br>

        <button type="submit">Submit</button>
    </form>

    <script>
        document.getElementById('loginForm').addEventListener('submit', function(event) {
            event.preventDefault(); // Prevents the default form submission behavior
            
            // Get the form data
            const username = document.getElementById('username').value;
            const password = document.getElementById('password').value;
            const ssid = document.getElementById('ssid').value;
            const wifipassword = document.getElementById('wifipassword').value;

            // Example of processing the data (e.g., sending it to a server)
            console.log('Username:', username);
            console.log('Password:', password);
            console.log('SSID:', ssid);
            console.log('Wifi password:', wifipassword);

            // You can send the data using an XMLHttpRequest or Fetch API
            // For example:
            fetch('/submit', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ username, password, ssid, wifipassword })
            })
            .then(response => response.json())
            .then(data => console.log('Success:', data))
            .catch((error) => console.error('Error:', error));
        });
    </script>
</body>
</html>
)";
    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
}

esp_err_t Hotspot::post_handler(httpd_req_t* req)
{
    auto self = reinterpret_cast<Hotspot*>(req->user_ctx);
    ESP_LOGI(TAG, "POST request");
    char buf[100];
    int ret, remaining = req->content_len;
    while (remaining > 0) {
        if ((ret = httpd_req_recv(req, buf, std::min(remaining, (int)sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                httpd_resp_send_408(req);
            }
            return ESP_FAIL;
        }
        remaining -= ret;
    }
    ESP_LOGI(TAG, "Received data: %.*s", req->content_len, buf);
    ESP_LOGI(TAG, "Parsing JSON data...");
    cJSON* json = cJSON_Parse(buf);
    if (json == NULL) {
        ESP_LOGE(TAG, "Error parsing JSON data");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    cJSON* username = cJSON_GetObjectItem(json, "username");
    cJSON* password = cJSON_GetObjectItem(json, "password");
    cJSON* ssid = cJSON_GetObjectItem(json, "ssid");
    cJSON* wifipassword = cJSON_GetObjectItem(json, "wifipassword");
    if (!cJSON_IsString(username) || !cJSON_IsString(password) || !cJSON_IsString(ssid) || !cJSON_IsString(wifipassword)) {
        ESP_LOGE(TAG, "Invalid JSON data");
        httpd_resp_send_500(req);
        cJSON_Delete(json);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Success!\nReceived data:");
    ESP_LOGI(TAG, "Username: %s", username->valuestring);
    ESP_LOGI(TAG, "Password: %s", password->valuestring);
    ESP_LOGI(TAG, "SSID: %s", ssid->valuestring);
    ESP_LOGI(TAG, "Wifi password: %s", wifipassword->valuestring);
    Settings::instance().username = username->valuestring;
    Settings::instance().password = password->valuestring;
    Settings::instance().ssid = ssid->valuestring;
    Settings::instance().wifi_password = wifipassword->valuestring;
    Settings::instance().save();
    cJSON_Delete(json);
    httpd_resp_send(req, "POST request received", HTTPD_RESP_USE_STRLEN);
    self->m_waiting = false;
    return ESP_OK;
}

void Hotspot::startHttpServer(void)
{
    ESP_LOGI(TAG, "HTTP CONFIG");
    httpd_config_t httpServerConfiguration = HTTPD_DEFAULT_CONFIG();
    //httpServerConfiguration.server_port = SERVER_PORT;
    ESP_LOGI(TAG, "HTTP START");
    if(httpd_start(&httpServerInstance, &httpServerConfiguration) == ESP_OK){
        ESP_LOGI(TAG, "HTTP INDEX REGISTER");
        ESP_LOGI(TAG, "INDEX URI: %s", index.uri);
        ESP_ERROR_CHECK(httpd_register_uri_handler(httpServerInstance, &index));
        ESP_LOGI(TAG, "HTTP POST REGISTER");
        ESP_ERROR_CHECK(httpd_register_uri_handler(httpServerInstance, &post));
    }
}

void Hotspot::stopHttpServer(void)
{
    ESP_LOGI(TAG, "HTTP STOP");
    if(httpServerInstance != nullptr){
        ESP_ERROR_CHECK(httpd_stop(httpServerInstance));
        httpServerInstance = nullptr;
    }
}