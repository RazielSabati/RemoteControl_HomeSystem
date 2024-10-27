#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <SPI.h>      // Library for SPI communication required for the LoRa module
#include <LoRa.h>     // LoRa library enabling communication with the LoRa module
#include <Arduino.h>
#include "../DisplayMenu/DisplayMenu.h"

class HomeCommunication {
private:
    String last_request;
public:
    HomeCommunication();  // Constructor
    void setupCommunication();  // Initialize LoRa communication
    void checkForAcknowledgment(bool &isWaitingForAck);
    void sendMessage(String message , DisplayMenu menu ,int menuType , int actionIndex );  // Send a message to the external system
    String receiveResponse();  // Receive confirmation/failure from the external system
};

#endif