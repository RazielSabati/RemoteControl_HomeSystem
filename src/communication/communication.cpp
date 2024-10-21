#include "communication.h"

#define ss 5
#define rst 14
#define dio0 2

HomeCommunication::HomeCommunication() {
    messageCount = 0;
}

void HomeCommunication::setupCommunication() {
    LoRa.setPins(ss, rst, dio0);
    if (!LoRa.begin(433E6)) {
        Serial.println("LoRa failed to start!");
        while (1);  // Stop the code in case of initialization failure
    }
    Serial.println("LoRa initialized successfully.");
}

void HomeCommunication::sendMessage() {

    // Construct the request message with the counter
    String message = "Requesting communication, message count: " + String(messageCount);
    
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();
    
    // Print the sent message
    Serial.println("Message sent: " + message);
    messageCount++;

    // Wait 10 seconds before sending another message
    delay(10000);
}