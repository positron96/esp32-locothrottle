#pragma once

#include <cstdint>
#include <cstdio>
#include <string>

class IPAddress {
public:
    IPAddress() = default;

    explicit IPAddress(uint32_t address)
        : address_(address) {}

    bool fromString(const char* text) {
        unsigned int octets[4];
        char trailing;
        if (text == nullptr ||
            std::sscanf(text, "%u.%u.%u.%u%c", &octets[0], &octets[1], &octets[2], &octets[3], &trailing) != 4 ||
            octets[0] > 255 || octets[1] > 255 || octets[2] > 255 || octets[3] > 255) {
            return false;
        }
        address_ = (octets[0] << 24) | (octets[1] << 16) | (octets[2] << 8) | octets[3];
        return true;
    }

    std::string toString() const {
        char text[16];
        std::snprintf(
            text,
            sizeof(text),
            "%u.%u.%u.%u",
            static_cast<unsigned>((address_ >> 24) & 0xff),
            static_cast<unsigned>((address_ >> 16) & 0xff),
            static_cast<unsigned>((address_ >> 8) & 0xff),
            static_cast<unsigned>(address_ & 0xff));
        return text;
    }

private:
    uint32_t address_ = 0;
};
