#pragma once

#include <cstddef>
#include <string>

class String {
public:
    String() = default;
    String(const char* value)
        : value_(value != nullptr ? value : "") {}

    bool isEmpty() const {
        return value_.empty();
    }

    const char* c_str() const {
        return value_.c_str();
    }

private:
    std::string value_;
};

constexpr int WL_CONNECTED = 3;
constexpr int WL_DISCONNECTED = 6;
constexpr int WIFI_SCAN_RUNNING = -1;
constexpr int WIFI_SCAN_FAILED = -2;

class WiFiClass {
public:
    int status() const {
        return connected_ ? WL_CONNECTED : WL_DISCONNECTED;
    }

    String SSID() const {
        return connected_ ? String(connected_ssid_.c_str()) : String();
    }

    String SSID(int network_index) const {
        static constexpr const char* network_names[] = {
            "NativeNetwork",
            "LayoutLab",
            "TestNetwork",
        };
        static constexpr int network_count = sizeof(network_names) / sizeof(network_names[0]);

        if (network_index < 0 || network_index >= network_count) {
            return String();
        }
        return String(network_names[network_index]);
    }

    int scanNetworks(bool async = false, bool = false) {
        if (async) {
            scan_running_ = true;
            return WIFI_SCAN_RUNNING;
        }
        return 3;
    }

    int scanComplete() {
        if (scan_running_) {
            scan_running_ = false;
            return 3;
        }
        return WIFI_SCAN_FAILED;
    }

    void scanDelete() {
        scan_running_ = false;
    }

    int begin(const char* ssid, const char* = nullptr) {
        if (ssid != nullptr) {
            connected_ssid_ = ssid;
            connected_ = true;
        }
        return connected_ ? WL_CONNECTED : WL_DISCONNECTED;
    }

private:
    bool scan_running_ = false;
    bool connected_ = false;
    std::string connected_ssid_;
};

inline WiFiClass WiFi;
