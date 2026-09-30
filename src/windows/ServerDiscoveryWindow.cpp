#include "ServerDiscoveryWindow.h"

#include <Arduino.h>

ServerDiscoveryWindow::ServerDiscoveryWindow(ServerDiscovery& discovery, DiscoveredServer& selectedServer)
    : discovery_(discovery), selectedServer_(selectedServer) {}

void ServerDiscoveryWindow::build() {
    statusLabel_ = lv_label_create(screen_);
    lv_obj_center(statusLabel_);
}

void ServerDiscoveryWindow::onEnter() {
    loadScreen();
    if (!built_) {
        build();
        built_ = true;
    }

    hasSelection_ = false;
    resultsPrinted_ = false;
    servers_.clear();

    lv_label_set_text(statusLabel_, "Scanning for\nWiThrottle servers...");
    Serial.println(F("[Discovery] Scanning for WiThrottle servers... (type 'c' + Enter to cancel)"));
    discovery_.begin();
}

void ServerDiscoveryWindow::printResults() {
    if (servers_.empty()) {
        Serial.println(F("[Discovery] No servers found"));
        lv_label_set_text(statusLabel_, "No servers found");
        return;
    }

    Serial.println(F("[Discovery] Servers found:"));
    lv_label_set_text_fmt(statusLabel_, "%u server(s) found\nuse Serial to select", servers_.size());
    for (size_t i = 0; i < servers_.size(); ++i) {
        Serial.printf("  [%u] %s (%s:%u)\n", static_cast<unsigned>(i), servers_[i].name.c_str(),
                       servers_[i].ip.toString().c_str(), servers_[i].port);
    }
    Serial.println(F("[Discovery] Type the number of the server to connect to."));
}

AppState ServerDiscoveryWindow::update(AppState current) {
    while (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) {
            continue;
        }

        if (line == "c" && !resultsPrinted_) {
            Serial.println(F("[Discovery] Cancelled by user"));
            discovery_.cancel();
            return AppState::Error;
        }

        if (!resultsPrinted_) {
            continue; // still scanning, selection not possible yet
        }

        int index = line.toInt();
        if (index < 0 || static_cast<size_t>(index) >= servers_.size()) {
            Serial.println(F("[Discovery] Invalid selection"));
            continue;
        }

        selectedServer_ = servers_[index];
        hasSelection_ = true;
    }

    if (hasSelection_) {
        return AppState::ServerConnecting;
    }

    if (!resultsPrinted_) {
        ServerDiscovery::Status status = discovery_.update();
        switch (status) {
            case ServerDiscovery::Status::Done:
                servers_ = discovery_.results();
                resultsPrinted_ = true;
                printResults();
                if (servers_.empty()) {
                    return AppState::Error;
                }
                break;

            case ServerDiscovery::Status::Failed:
            case ServerDiscovery::Status::Cancelled:
                return AppState::Error;

            case ServerDiscovery::Status::Scanning:
            default:
                break;
        }
    }

    return current;
}
