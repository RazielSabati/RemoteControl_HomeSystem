#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <SPI.h>      // Library for SPI communication required for the LoRa module
#include <LoRa.h>     // LoRa library enabling communication with the LoRa module
#include <Arduino.h>
#include "../DisplayMenu/DisplayMenu.h"

class HomeCommunication {
private:
    String last_request_code;
    String last_request;
    DisplayMenu menu;

    const unsigned long RETRY_INTERVAL = 1000; // Time between retries in ms
    const uint8_t MAX_RETRIES = 5;            // Maximum number of retries
    uint8_t retry_count;
    
    
    // Pin definitions
    static const uint8_t LORA_SS_PIN = 5;
    static const uint8_t LORA_RST_PIN = 14;
    static const uint8_t LORA_DIO0_PIN = 2;
    
    // Buffer for receiving messages
    static const uint8_t MAX_MESSAGE_LENGTH = 32;
    char receive_buffer[MAX_MESSAGE_LENGTH];

public:
    HomeCommunication() : retry_count(0) {}

    bool setupCommunication();  // Initialize LoRa communication
    bool checkForAcknowledgment(bool& isWaitingForAck, DisplayMenu& menu);
    bool sendMessage(const String& message, DisplayMenu& menu, int menuType, int actionIndex );  // Send a message to the external system
};

#endif