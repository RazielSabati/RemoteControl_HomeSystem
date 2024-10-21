#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <SPI.h>      // Library for SPI communication required for the LoRa module
#include <LoRa.h>     // LoRa library enabling communication with the LoRa module

class HomeCommunication {
    int messageCount;  // Message counter
public:
    HomeCommunication();  // Constructor
    void setupCommunication();  // Initialize LoRa communication
    void sendMessage();  // Send a message to the external system
    String receiveResponse();  // Receive confirmation/failure from the external system
};

#endif