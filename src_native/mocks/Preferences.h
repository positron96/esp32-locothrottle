#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <WiFi.h>

class Preferences {
public:
    bool begin(const char*, bool = false, const char* = nullptr) {
        return true;
    }

    String getString(const char* key, const char* default_value = "") const {
        if (key != nullptr && std::strcmp(key, "host") == 0) {
            return String("192.168.1.42");
        }
        return String(default_value);
    }

    uint16_t getUShort(const char* key, uint16_t default_value = 0) const {
        if (key != nullptr && std::strcmp(key, "port") == 0) {
            return 12090;
        }
        return default_value;
    }

    size_t putString(const char*, const char* value) {
        return value == nullptr ? 0 : std::strlen(value);
    }

    size_t putUShort(const char*, uint16_t) {
        return sizeof(uint16_t);
    }

    void end() {}
};
