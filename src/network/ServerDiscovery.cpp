#include "ServerDiscovery.h"

ServerDiscovery::~ServerDiscovery() {
    endSearch();
}

void ServerDiscovery::begin() {
    if (!mdnsStarted_) {
        mdnsStarted_ = (mdns_init() == ESP_OK);
        if (mdnsStarted_) {
            mdns_hostname_set("withremote");
        }
    }

    endSearch();
    servers_.clear();

    if (!mdnsStarted_) {
        status_ = Status::Failed;
        return;
    }

    search_ = mdns_query_async_new(nullptr, "_withrottle", "_tcp", MDNS_TYPE_PTR, SCAN_TIMEOUT_MS, MAX_QUERY_RESULTS,
                                    nullptr);
    status_ = search_ ? Status::Scanning : Status::Failed;
}

ServerDiscovery::Status ServerDiscovery::update() {
    if (status_ != Status::Scanning) {
        return status_;
    }

    mdns_result_t* results = nullptr;
    uint8_t numResults = 0;
    // timeout=0: non-blocking poll for whether the (already-running) async
    // query has finished yet.
    bool finished = mdns_query_async_get_results(search_, 0, &results, &numResults);
    if (finished) {
        collectResults(results);
        mdns_query_results_free(results);
        endSearch();
        status_ = Status::Done;
    }

    return status_;
}

void ServerDiscovery::cancel() {
    endSearch();
    status_ = Status::Cancelled;
}

void ServerDiscovery::endSearch() {
    if (search_) {
        mdns_query_async_delete(search_);
        search_ = nullptr;
    }
}

void ServerDiscovery::collectResults(mdns_result_t* results) {
    for (mdns_result_t* r = results; r != nullptr && !servers_.full(); r = r->next) {
        DiscoveredServer server;
        server.name = r->instance_name ? r->instance_name : (r->hostname ? r->hostname : "");
        server.port = r->port;

        for (mdns_ip_addr_t* addr = r->addr; addr != nullptr; addr = addr->next) {
            if (addr->addr.type == MDNS_IP_PROTOCOL_V4) {
                server.ip = IPAddress(addr->addr.u_addr.ip4.addr);
                break;
            }
        }

        servers_.push_back(server);
    }
}
