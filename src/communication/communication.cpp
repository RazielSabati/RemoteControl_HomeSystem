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


void HomeCommunication::sendMessage(String message) {
    Serial.print("Sending message: ");
    Serial.println(message);

    LoRa.beginPacket();       
    LoRa.print(message);     
    LoRa.endPacket();         
    delay(1000);              

}


void HomeCommunication::checkForAcknowledgment(bool &isWaitingForAck) {
        int packetSize = LoRa.parsePacket();  // בדיקת קבלת הודעה חדשה
        if (packetSize) {
            String receivedMessage = "";
            while (LoRa.available()) {
                receivedMessage += (char)LoRa.read();  // קריאת ההודעה שהתקבלה
            }

            // בדיקה אם ההודעה היא אישור (ACK)
            if (receivedMessage == "ACK") {
                Serial.println("Acknowledgment received.");
                isWaitingForAck = false;  // קיבלנו אישור, אין צורך להמתין יותר
            }
        }
}



