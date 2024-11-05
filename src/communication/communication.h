#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <SPI.h>      // Library for SPI communication required for the LoRa module
#include <LoRa.h>     // LoRa library enabling communication with the LoRa module
#include <Arduino.h>
#include "../DisplayMenu/DisplayMenu.h"

class HomeCommunication {
private:
    int last_request_code;
    String last_request;
    uint8_t last_request_uint8_t;
    const uint8_t ERROR_CHECK_MASK = 0x0F;

    DisplayMenu menu;
    
    // Pin definitions
    static const uint8_t LORA_SS_PIN = 5;
    static const uint8_t LORA_RST_PIN = 14;
    static const uint8_t LORA_DIO0_PIN = 2;
    

public:
    HomeCommunication() {}
    bool setupCommunication();  // Initialize LoRa communication
    bool checkForAcknowledgment(bool& isWaitingForAck, DisplayMenu& menu);
    bool sendMessage( DisplayMenu& menu, int menuType, int actionIndex );  // Send a message to the external system
    String getLastRequest();
};

#endif