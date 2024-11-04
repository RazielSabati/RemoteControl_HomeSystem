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
    LoRa.setSpreadingFactor(12);
    LoRa.setSignalBandwidth(62.5E3);
    LoRa.setCodingRate4(5);
    LoRa.setPreambleLength(8);
    LoRa.setTxPower(20);
    // LoRa.enableCrc();
    
    Serial.println(F("LoRa initialized successfully."));
    return true;
}

bool HomeCommunication::sendMessage(DisplayMenu& menu, int menuType, int actionIndex) {
        last_request_code = menuType * 10 + actionIndex;;
        last_request = menu.getData(menuType, actionIndex);
        
        Serial.print(F("Sending message: "));
        Serial.println(last_request);
        
         // Input validation
    if (menuType < 0 || menuType > 1 || actionIndex < 0 || actionIndex > 5) {
        Serial.println(F("Invalid input values!"));
        return false;
    }

    
    // Create message byte
    // First bit is menuType (0 or 1)
    // Next 3 bits are actionIndex (0-5)
    // Last 4 bits are error check
    uint8_t messageByte = (menuType & 0x01) << 7;    // Put menuType in most significant bit
    messageByte |= (actionIndex & 0x07) << 4;        // Put actionIndex in next 3 bits
    
    // Create error check using the first 4 bits
    uint8_t errorCheck = ((messageByte >> 4) & 0x0F);
    errorCheck = errorCheck ^ ERROR_CHECK_MASK;       // XOR with mask for error checking
    messageByte |= errorCheck & 0x0F;                // Add error check in least significant 4 bits
    
    last_request_uint8_t = messageByte;
    
    // Debug print
    Serial.print(F("Sending byte: 0b"));
    for (int i = 7; i >= 0; i--) {
        Serial.print((messageByte >> i) & 0x01);
    }
    Serial.println();

    // Send using LoRa
    LoRa.beginPacket();
    LoRa.write(messageByte);  // Send single byte
    bool success = LoRa.endPacket();
    
    if (!success) {
        Serial.println(F("Failed to send message"));
        return false;
    }

    menu.displayConfirmationMessage(last_request + " sent");
    
    delay(10);  // Short delay for stability
    return true;
}

bool HomeCommunication::checkForAcknowledgment(bool& isWaitingForAck, DisplayMenu& menu) { 
        int packetSize = LoRa.parsePacket();
       
        // Check if we received exactly one byte (8 bits)
        if (packetSize != 1) {
            // Serial.println(F("Invalid packet size received."));
            return false;  // Exit the function if packet size is not exactly 1 byte
        }

        uint8_t receivedByte = LoRa.read();  // Read the single byte message

        Serial.print(F("Received byte: 0b"));
        for (int i = 7; i >= 0; i--) {
            Serial.print((receivedByte >> i) & 0x01);
        }
        Serial.println();

        if (receivedByte == last_request_uint8_t) {
            Serial.println(F("Acknowledgment received"));
            isWaitingForAck = false;
            retry_count = 0;

            menu.displayConfirmationMessage(last_request + " done");
            return true;
        }
        else
            return false;
}


