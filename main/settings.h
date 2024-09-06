#pragma once

#include <string>
#include <memory>

namespace nvs {
    class NVSHandle;
}

class Settings {

public:
    static Settings &instance();
    void save();
    std::string ssid;
    std::string wifi_password;
    std::string username;
    std::string password;

private:
    Settings();

private:
    //std::unique_ptr<nvs::NVSHandle> handle;
};