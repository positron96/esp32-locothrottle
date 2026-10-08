#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>

#include <Preferences.h>
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
            save_server();
        } else {
            clear_server_connection();
        }
        return is_connected;
    }

    bool get_saved_server(char* host, size_t host_size, uint16_t& port) {
        if (host == nullptr || host_size == 0) {
            return false;
        }

        host[0] = '\0';
        port = 0;

        Preferences preferences;
        if (!preferences.begin("withremote", true)) {
            std::fprintf(stderr, "Failed to open saved server preferences\n");
            return false;
        }

        const String saved_host = preferences.getString("host", "");
        const uint16_t saved_port = preferences.getUShort("port", 0);
        preferences.end();

        const size_t saved_host_length = std::strlen(saved_host.c_str());
        if (saved_host_length == 0 || saved_port == 0 || saved_host_length >= host_size) {
            return false;
        }

        std::memcpy(host, saved_host.c_str(), saved_host_length + 1);
        port = saved_port;
        return true;
    }

    static AppState& get_instance() {
        static AppState instance;
        return instance;
    }

private:
    void save_server() {
        Preferences preferences;
        if (!preferences.begin("withremote", false)) {
            std::fprintf(stderr, "Failed to open saved server preferences for writing\n");
            return;
        }

        const size_t host_bytes = preferences.putString("host", server_host);
        const size_t port_bytes = preferences.putUShort("port", server_port);
        preferences.end();

        if (host_bytes == 0 || port_bytes == 0) {
            std::fprintf(stderr, "Failed to save the server address and port\n");
        }
    }

};
