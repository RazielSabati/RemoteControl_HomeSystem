#include "communication.h"

#define ss 5
#define rst 14
#define dio0 2

HomeCommunication::HomeCommunication() {}

void HomeCommunication::setupCommunication() {
    // Initialize the LoRa module
    LoRa.setPins(ss, rst, dio0);
    if (!LoRa.begin(433E6)) {
        Serial.println("LoRa failed to start!");
        while (1);  // Stop the code in case of initialization failure
    }
    Serial.println("LoRa initialized successfully.");
}


void HomeCommunication::sendMessage(String message , DisplayMenu menu ,int menuType , int actionIndex ) {
    last_request = message;

    Serial.print("Sending message: ");
    Serial.println(menu.getData(menuType, actionIndex));
    Serial.println(message);


    LoRa.beginPacket();       
    LoRa.print(message);     
    LoRa.endPacket();                    
}


void HomeCommunication::checkForAcknowledgment(bool &isWaitingForAck) {
        int packetSize = LoRa.parsePacket();  // read packets
        if (packetSize) {
            String receivedMessage = "";
            while (LoRa.available()) {
                receivedMessage += (char)LoRa.read();  // read the incoming message
            }

            Serial.println("Received message: " + receivedMessage);
            
            last_request = "ACK" + last_request;
            // Check if the received message is an acknowledgment
            if (receivedMessage == last_request){
                Serial.println("Acknowledgment received.");
                isWaitingForAck = false;  // Stop waiting for acknowledgment
            }
        }
}



