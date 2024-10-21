#include "./communication/communication.h"

HomeCommunication homeCom;

void setup() {
    // Initialize home system configuration
    Serial.begin(9600);
    while (!Serial);          // Wait until Serial is ready

    homeCom.setupCommunication();
    Serial.println("Home system ready to send messages.");
}

void loop() {
    homeCom.sendMessage();
}