#pragma once

#include <string>

class Settings {

public:
    static Settings &instance();
    std::string ssid;
    std::string wifi_password;
    std::string username;
    std::string password;
private:
    Settings() = default;
    ~Settings() = default;
};