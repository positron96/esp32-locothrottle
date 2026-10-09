#pragma once

#include <cstdint>

#include <IPAddress.h>

class WiFiClient {
public:
    void stop() {
        connected_ = false;
    }

    bool connect(const IPAddress& address, uint16_t port, int32_t) {
        connected_ = port != 0 && address.toString() != "0.0.0.0";
        return connected_;
    }

private:
    bool connected_ = false;
};
