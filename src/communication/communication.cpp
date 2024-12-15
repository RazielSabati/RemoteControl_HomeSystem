#include "communication.h"

HomeCommunication::HomeCommunication() {
    memset(iv, 0, sizeof(iv));
    memset(plaintext, 0, sizeof(plaintext));
    memset(encrypted, 0, sizeof(encrypted));
    memset(decrypted, 0, sizeof(decrypted));
    rollingCode = -1;
}

bool HomeCommunication::setupCommunication() {
    // Initialize SPI first
    SPI.begin();

    LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);

    unsigned long start = millis();
    while (!LoRa.begin(433E6)) {
        if (millis() - start > 5000) {
            Serial.println(F("LoRa initialization failed!"));
            return false;
        }
        delay(100);
    }

    LoRa.setSignalBandwidth(31.25E3);
    LoRa.setSpreadingFactor(13);
    LoRa.setCodingRate4(9);
    LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN); // Specify output pin

    Serial.println(F("LoRa initialized successfully."));
    return true;
}

bool HomeCommunication::sendMessage(DisplayMenu& menu, int menuType, int actionIndex) {

    last_request = menu.getData(menuType, actionIndex);

    randomNumber = random(0,16); // Generate a random number between 0 and 15

    rollingCode++; // Increment the rolling code

    message = (menuType & 0x01) << 7;    // Put menuType in most significant bit
    message |= (actionIndex & 0x07) << 4;        // Put actionIndex in next 3 bits
    message |= (randomNumber & 0x0F) ;       // Put randomNumber in next 4 bits

    last_request_code = message ^ 0xF0; // Save the last request code


    // Prepare data packet with bounds checking
    memset(plaintext, 0, sizeof(plaintext));  // Clear data array first
    plaintext[0] = message; // Put message in first byte
    plaintext[15] = message ^ 0xF0; // Put message XOR 0xF0 in last byte
    
    // Put the rolling code in the next 4 bytes
    plaintext[1] =  (rollingCode >> 24) & 0xFF; 
    plaintext[2] = (rollingCode >> 16) & 0xFF;     
    plaintext[3] = (rollingCode >> 8) & 0xFF;     
    plaintext[4] = rollingCode & 0xFF;     

    for (int i = 5; i < 15; i++) {
        plaintext[i] = (uint8_t)random(0, 256);
    }

    generateRandomIV(); // Generate a random IV for encryption

    aesLib.encrypt(plaintext, 16, encrypted, key, 128, iv_for_en); // Encrypt the data

    // שליחת הודעה עם ה-IV וה-Rolling Code
    LoRa.beginPacket(); 
    LoRa.write(iv, 16);            // sending the IV
    LoRa.write(encrypted, 16); // sending the encrypted message

    bool success = LoRa.endPacket();  // Use async mode
    if (!success) {
        Serial.println(F("Failed to send packet"));
        return false;
    }

    return true;

}


bool HomeCommunication::checkForAcknowledgment(bool& isWaitingForAck, DisplayMenu& menu) {

    int packetSize = LoRa.parsePacket();

    if (packetSize == 32) {

        LoRa.readBytes(buffer, 32);// Read the incoming message


        memcpy(iv, buffer, 16); // Extract the IV (first 8 bytes)
        memcpy(encrypted, buffer + 16, 32);// Extract the encrypted message (next 8 bytes)


        // Decrypt the message using AES with the received IV
        aesLib.decrypt(encrypted, 16, decrypted, key, 128, iv);

        uint32_t rollingCodeReceived = 
            (decrypted[1] << 24) | 
            (decrypted[2] << 16) | 
            (decrypted[3] << 8) | 
            decrypted[4];

        if(rollingCodeReceived <= rollingCode) {
            Serial.println(F("Received message with the same rolling code."));
            return false; // mabey we need to do something else here*******************************
        }

        rollingCode = rollingCodeReceived; // Update the rolling code

        message = decrypted[0]; // Extract the message from the decrypted data

        
        if( (message ^ 0xF0) == last_request_code){
            Serial.println(F("Acknowledgment received"));
            menu.displayConfirmationMessage(last_request + " done", 1);
            isWaitingForAck = false;
            return true;
        }
        else
        {
            Serial.println(F("Invalid acknowledgment"));
            menu.displayConfirmationMessage("Invalid acknowledgment", 1);
            return false;
        }
    }
    else
        return false;
}

bool HomeCommunication::send_keep_alive() {
    int size = random(28, 33);
    uint8_t data[size];
    for (int i = 0; i < size; i++) {
        data[i] = random(0, 256);
    }

    LoRa.beginPacket();
    LoRa.write(data, size);
    bool success = LoRa.endPacket();
    if (success) {
        Serial.print(F("Sent message: "));
    } else {
        Serial.println(F("Failed to send message"));
    }
    return success;
}



void HomeCommunication::generateRandomIV() {
    for (int i = 0; i < 16; i++) {
        iv[i] = random(0, 256);
    }
    // Copy the IV to iv_for_en
    memcpy(iv_for_en, iv, 16);
}



String HomeCommunication::getLastRequest() {
    return last_request;
}

