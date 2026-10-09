#pragma once

#include <cstddef>
#include <cstdint>

constexpr int ESP_OK = 0;
constexpr int MDNS_TYPE_PTR = 12;
constexpr int MDNS_IP_PROTOCOL_V4 = 0;

struct mdns_ip_addr_t {
    mdns_ip_addr_t* next = nullptr;
    struct {
        int type = MDNS_IP_PROTOCOL_V4;
        union {
            struct {
                uint32_t addr;
            } ip4;
        } u_addr = {};
    } addr;
};

struct mdns_result_t {
    mdns_result_t* next = nullptr;
    const char* instance_name = nullptr;
    const char* hostname = nullptr;
    uint16_t port = 0;
    mdns_ip_addr_t* addr = nullptr;
};

struct mdns_search_once_t {
    bool complete = false;
};

inline int mdns_init() {
    return ESP_OK;
}

inline int mdns_hostname_set(const char*) {
    return ESP_OK;
}

inline mdns_search_once_t* mdns_query_async_new(
    const char*,
    const char*,
    const char*,
    int,
    uint32_t,
    size_t,
    void*) {
    return new mdns_search_once_t;
}

inline bool mdns_query_async_get_results(
    mdns_search_once_t* search,
    uint32_t,
    mdns_result_t** results,
    uint8_t* result_count) {
    if (search == nullptr || search->complete) {
        return false;
    }
    search->complete = true;

    static mdns_ip_addr_t first_address;
    static mdns_ip_addr_t second_address;
    first_address.addr.type = MDNS_IP_PROTOCOL_V4;
    first_address.addr.u_addr.ip4.addr = 0xC0A8012C;
    first_address.next = nullptr;
    second_address.addr.type = MDNS_IP_PROTOCOL_V4;
    second_address.addr.u_addr.ip4.addr = 0xC0A8012D;
    second_address.next = nullptr;

    static mdns_result_t first_server;
    static mdns_result_t second_server;
    first_server.next = &second_server;
    first_server.instance_name = "MockThrottle";
    first_server.hostname = "mock-throttle";
    first_server.port = 12090;
    first_server.addr = &first_address;
    second_server.next = nullptr;
    second_server.instance_name = "LayoutLab";
    second_server.hostname = "layout-lab";
    second_server.port = 12091;
    second_server.addr = &second_address;

    *results = &first_server;
    *result_count = 2;
    return true;
}

inline void mdns_query_results_free(mdns_result_t*) {}

inline void mdns_query_async_delete(mdns_search_once_t* search) {
    delete search;
}
