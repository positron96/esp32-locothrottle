#include <Arduino.h>
#include <WiFiClient.h>

#include "AppState.h"
#include "AppStateMachine.h"
#include "inputs/SerialThrottleInputs.h"
#include "network/ServerDiscovery.h"
#include "network/WiFiSetup.h"
#include "windows/ErrorWindow.h"
#include "windows/MainControlWindow.h"
#include "windows/ServerConnectWindow.h"
#include "windows/ServerDiscoveryWindow.h"
#include "windows/WifiConnectWindow.h"

namespace {

WiFiSetup wifiSetup;
ServerDiscovery serverDiscovery;
DiscoveredServer selectedServer;
SerialThrottleInputs throttleInputs;
WiFiClient serverLink;

WifiConnectWindow wifiConnectWindow(wifiSetup);
ServerDiscoveryWindow serverDiscoveryWindow(serverDiscovery, selectedServer);
ServerConnectWindow serverConnectWindow(selectedServer, serverLink);
MainControlWindow mainControlWindow(throttleInputs, serverLink);
ErrorWindow errorWindow;

AppStateMachine stateMachine(AppState::WifiConnecting, wifiConnectWindow, serverDiscoveryWindow, serverConnectWindow,
                             mainControlWindow, errorWindow);

} // namespace

void setup() {
    Serial.begin(115200);
    Serial.println("Starting WiThRemote");

    stateMachine.begin();
}

void loop() {
    stateMachine.update();
}
