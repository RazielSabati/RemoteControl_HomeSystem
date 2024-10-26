#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <SPI.h>      // Library for SPI communication required for the LoRa module
#include <LoRa.h>     // LoRa library enabling communication with the LoRa module
#include <Arduino.h>

class HomeCommunication {
public:
    HomeCommunication();  // Constructor
    void setupCommunication();  // Initialize LoRa communication
    void checkForAcknowledgment(bool &isWaitingForAck);
    void checkButtonsAndSendMessage();  // Check button presses and send corresponding messages
    void sendMessage(String message);  // Send a message to the external system
    String receiveResponse();  // Receive confirmation/failure from the external system
};

#endif