#pragma once

#include <mdns.h>

#include <IPAddress.h>
#include <etl/string.h>
#include <etl/vector.h>

struct DiscoveredServer {
    etl::string<64> name;
    IPAddress ip;
    uint16_t port = 0;
};

constexpr size_t MAX_DISCOVERED_SERVERS = 8;
using DiscoveredServerList = etl::vector<DiscoveredServer, MAX_DISCOVERED_SERVERS>;

// Discovers WiThrottle servers on the local network via mDNS.
//
// Non-blocking: begin() starts an asynchronous PTR query (using the
// lower-level ESP-IDF mdns API, since the Arduino ESPmDNS wrapper only
// offers a blocking queryService()). update() must be polled every loop()
// iteration to check for completion, and cancel() aborts an in-progress
// scan.
//
// This only performs network discovery (finding host/IP/port); it does not
// speak the WiThrottle protocol itself.
class ServerDiscovery {
public:
    enum class Status {
        Scanning,
        Done,
        Cancelled,
        Failed,
    };

    ~ServerDiscovery();

    // Starts (or restarts) a scan for _withrottle._tcp services. Does not block.
    void begin();

    // Polls for completion. Call every loop() iteration. Returns the current status.
    Status update();

    // Aborts an in-progress scan.
    void cancel();

    Status status() const { return status_; }

    // Valid once status() == Done.
    const DiscoveredServerList& results() const { return servers_; }

private:
    void collectResults(mdns_result_t* results);
    void endSearch();

    static constexpr uint32_t SCAN_TIMEOUT_MS = 3000;
    static constexpr size_t MAX_QUERY_RESULTS = 20;

    bool mdnsStarted_ = false;
    mdns_search_once_t* search_ = nullptr;
    Status status_ = Status::Failed;
    DiscoveredServerList servers_;
};
