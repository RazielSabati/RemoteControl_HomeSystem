#include "communication.h"

bool HomeCommunication::setupCommunication() {
    LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
    
    // Try to initialize LoRa with timeout
    unsigned long start = millis();
    while (!LoRa.begin(433E6)) {
        if (millis() - start > 5000) {
            Serial.println(F("LoRa initialization failed!"));
            return false;
        }
        delay(100);
    }
    
   // הגדרות למרחק מקסימלי
    LoRa.setSpreadingFactor(12); //שידור איטי יותר, אך החוסן והטווח גדלים
    LoRa.setSignalBandwidth(31.25E3); // רוחב פס מאוזן יותר
    LoRa.setCodingRate4(8);
    LoRa.setPreambleLength(8);        // אורך preamble סטנדרטי
    LoRa.setTxPower(20);
    LoRa.enableCrc();
    
    Serial.println(F("LoRa initialized successfully."));
    return true;
}

bool HomeCommunication::sendMessage(const String& message, DisplayMenu& menu, int menuType, int actionIndex) {
        if (message.length() >= MAX_MESSAGE_LENGTH) {
            Serial.println(F("Message too long!"));
            return false;
        }
        
        last_request_code = message;
        last_request = menu.getData(menuType, actionIndex);
        
        Serial.print(F("Sending message: "));
        Serial.println(last_request);
        
        LoRa.beginPacket();
        LoRa.print(message);
        bool success = LoRa.endPacket();
        
        if (!success) {
            Serial.println(F("Failed to send message"));
            return false;
        }
        
        delay(10); // Short delay for stability
        return true;
    }

bool HomeCommunication::checkForAcknowledgment(bool& isWaitingForAck, DisplayMenu& menu) { 
        int packetSize = LoRa.parsePacket();
        if (packetSize == 0) 
            return false;
        
        
        // Clear buffer
        memset(receive_buffer, 0, MAX_MESSAGE_LENGTH);
        
        // Read message with bounds checking
        size_t i = 0;
        while (LoRa.available() && i < MAX_MESSAGE_LENGTH - 1) {
            receive_buffer[i++] = (char)LoRa.read();
        }
        receive_buffer[i] = '\0';

        
        String receivedMessage = String(receive_buffer);
        
        Serial.println(receivedMessage);

        if (receivedMessage == last_request_code) {
            Serial.println(F("Acknowledgment received"));
            isWaitingForAck = false;
            retry_count = 0;

            menu.displayConfirmationMessage(last_request + " done");
            return true;
        }
        else
            return false;
}