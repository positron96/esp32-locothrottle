#pragma once

#include <WString.h>
#include <WiFiManager.h>

// Connects to WiFi, using a previously saved network if available, or
// falling back to the WiFiManager configuration portal otherwise.
//
// Non-blocking: begin() kicks off the connection attempt and update() must
// be called every loop() iteration to progress it. cancel() lets the user
// abort while it is in progress (e.g. from a UI button or Serial command).
class WiFiSetup {
public:
    enum class Status {
        Connecting,   // Trying saved credentials.
        ConfigPortal, // Saved credentials failed; portal is up for the user to configure WiFi.
        Connected,
        Cancelled,
        Failed,
    };

    // Starts (or restarts) a connection attempt. Does not block.
    void begin();

    // Advances the connection attempt. Call every loop() iteration.
    // Returns the current status.
    Status update();

    // Aborts an in-progress connection attempt/config portal.
    void cancel();

    Status status() const { return status_; }
    bool isConnected() const { return status_ == Status::Connected; }

    // Name of the configuration portal's access point, useful for status
    // messages while status() == ConfigPortal.
    const String& configPortalApName() const { return apName_; }

private:
    static constexpr unsigned long SAVED_CREDENTIALS_TIMEOUT_MS = 15000;

    Status status_ = Status::Failed;
    unsigned long attemptStartMs_ = 0;
    String apName_ = "WiThRemote";
    WiFiManager wifiManager_;
};


