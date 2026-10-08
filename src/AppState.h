#pragma once

#include <cstdint>
#include <cstring>

#include <WiFiClient.h>

class AppState {

public:
    bool is_connected = false;
    char server_host[64] = {};
    uint16_t server_port = 0;

    WiFiClient server_client;

    bool is_server_connected() const {
        return is_connected;
    }

    void clear_server_connection() {
        server_client.stop();
        is_connected = false;
        server_host[0] = '\0';
        server_port = 0;
    }

    bool connect_server(const char* host, uint16_t port) {
        IPAddress address;
        if (host == nullptr || port == 0 || !address.fromString(host)) {
            clear_server_connection();
            return false;
        }

        server_client.stop();
        is_connected = server_client.connect(address, port, 5000);
        if (is_connected) {
            std::strncpy(server_host, host, sizeof(server_host) - 1);
            server_host[sizeof(server_host) - 1] = '\0';
            server_port = port;
        } else {
            clear_server_connection();
        }
        return is_connected;
    }

    static AppState& get_instance() {
        static AppState instance;
        return instance;
    }

};
