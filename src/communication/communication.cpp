#include "communication.h"

HomeCommunication::HomeCommunication() {}

bool HomeCommunication::setupCommunication() {
    LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);

    unsigned long start = millis();
    while (!LoRa.begin(433E6)) {
        if (millis() - start > 5000) {
            Serial.println(F("LoRa initialization failed!"));
            return false;
        }
        delay(100);
    }

    LoRa.setSignalBandwidth(62.5E3);
    LoRa.setSpreadingFactor(12);
    LoRa.setCodingRate4(8);
    LoRa.setTxPower(20);

    Serial.println(F("LoRa initialized successfully."));
    return true;
}

uint8_t HomeCommunication:: lfsrScramble(uint8_t unscrambled) {
        // הוספת XOR עם מספר קבוע
    uint8_t key = 0x3A;  // מספר קבוע לערבוב
    unscrambled ^= key;

    // החלפה בין הביט השני והשלישי
    bool bit1 = (unscrambled >> 1) & 0x01;
    bool bit2 = (unscrambled >> 2) & 0x01;
    unscrambled ^= (-bit1 ^ unscrambled) & (1 << 2);  // החלפה
    unscrambled ^= (-bit2 ^ unscrambled) & (1 << 1);

    // החלפה בין הביט הרביעי והשמיני
    bool bit3 = (unscrambled >> 3) & 0x01;
    bool bit7 = (unscrambled >> 7) & 0x01;
    unscrambled ^= (-bit3 ^ unscrambled) & (1 << 7);
    unscrambled ^= (-bit7 ^ unscrambled) & (1 << 3);

    // החלפה בין הביט החמישי והשביעי
    bool bit4 = (unscrambled >> 4) & 0x01;
    bool bit6 = (unscrambled >> 6) & 0x01;
    unscrambled ^= (-bit4 ^ unscrambled) & (1 << 6);
    unscrambled ^= (-bit6 ^ unscrambled) & (1 << 4);

    return unscrambled;
}

uint8_t HomeCommunication:: lfsrUnscramble(uint8_t scrambled) {
    uint8_t key = 0x3A;  // אותו מספר קבוע עבור ה-XOR

    // החלפה בין הביט החמישי והשביעי
    bool bit4 = (scrambled >> 4) & 0x01;
    bool bit6 = (scrambled >> 6) & 0x01;
    scrambled ^= (-bit4 ^ scrambled) & (1 << 6);
    scrambled ^= (-bit6 ^ scrambled) & (1 << 4);

    // החלפה בין הביט הרביעי והשמיני
    bool bit3 = (scrambled >> 3) & 0x01;
    bool bit7 = (scrambled >> 7) & 0x01;
    scrambled ^= (-bit3 ^ scrambled) & (1 << 7);
    scrambled ^= (-bit7 ^ scrambled) & (1 << 3);

    // החלפה בין הביט השני והשלישי
    bool bit1 = (scrambled >> 1) & 0x01;
    bool bit2 = (scrambled >> 2) & 0x01;
    scrambled ^= (-bit1 ^ scrambled) & (1 << 2);
    scrambled ^= (-bit2 ^ scrambled) & (1 << 1);

    // הוספת XOR עם אותו מספר קבוע כדי להחזיר למצב המקורי
    scrambled ^= key;

    return scrambled;
}



bool HomeCommunication::sendMessage(DisplayMenu& menu, int menuType, int actionIndex) {
    last_request = menu.getData(menuType, actionIndex);
    Serial.println(F("menuType: "));
    Serial.println(menuType);
    Serial.println(F("actionIndex: "));
    Serial.println(actionIndex);

    randomNumber = random(0,16); // Generate a random number between 0 and 15

    uint8_t message = (menuType & 0x01) << 7;    // Put menuType in most significant bit
    message |= (actionIndex & 0x07) << 4;        // Put actionIndex in next 3 bits
    message |= (randomNumber & 0x0F) ;       // Put randomNumber in next 4 bits

    last_request_code = message ^ 0xF0; // Save the last request code
    
    Serial.print(F(" message before scramble: "));   
    Serial.println(message, BIN);

    uint8_t scrambled_message = lfsrScramble(message);

    Serial.print(F(" message after scramble: "));   
    Serial.println(scrambled_message, BIN);

    
   
    LoRa.beginPacket();

    // שליחת ההודעה פעמיים רצוף
    LoRa.write(scrambled_message);
    LoRa.write(scrambled_message);
    
    bool success = LoRa.endPacket();
    if (success) {
        Serial.print(F("Sent message: "));
        Serial.println(message, BIN);
    } else {
        Serial.println(F("Failed to send message"));
    }

    return success;
}

bool HomeCommunication::checkForAcknowledgment(bool& isWaitingForAck, DisplayMenu& menu) {
    int packetSize = LoRa.parsePacket();

    if (packetSize == 2) {
        uint8_t buffer[2];
        LoRa.readBytes(buffer, 2);

        // שליפת ההודעה מתוך הביטים שקיבלנו
        uint8_t receivedMessage1 = buffer[0];
        uint8_t receivedMessage2 = buffer[1];

        // בדיקה אם שתי ההודעות שהתקבלו זהות
        if (receivedMessage1 != receivedMessage2) {
            Serial.println(F("Message mismatch error."));
            return false;
        }

        uint8_t scrambledMessage = receivedMessage1;

        Serial.print(F("Received scrambled message: 0b"));
        Serial.println(scrambledMessage, BIN);

        // החזרת הסקרימבול המקורי (דיסקרימבול) 
        uint8_t decryptedMessage = lfsrUnscramble(scrambledMessage); 

        if( decryptedMessage ^ 0xF0 == last_request_code) {
            Serial.println(F("Acknowledgment received"));
            menu.displayConfirmationMessage(last_request + " done", 0);
            isWaitingForAck = false;
            return true;
        } else
         {
            Serial.println(F("Invalid acknowledgment"));
            menu.displayConfirmationMessage("Invalid acknowledgment", 1);
            return false;
        }
    } 
    else 
        return false;
}

String HomeCommunication::getLastRequest() {
    return last_request;
}


bool HomeCommunication::send_keep_alive() {
    int menuType = random(0,2); // Generate a random number between 0 and 1
    int actionIndex = random(6,8); // Generate a random number between 6 and 7
    randomNumber = random(0,16); // Generate a random number between 0 and 15

    uint8_t message = (menuType & 0x01) << 7;    // Put menuType in most significant bit
    message |= (actionIndex & 0x07) << 4;        // Put actionIndex in next 3 bits
    message |= (randomNumber & 0x0F) ;       // Put randomNumber in next 4 bits

    uint8_t scrambled_message = lfsrScramble(message);
   
    LoRa.beginPacket();

    // שליחת ההודעה פעמיים רצוף
    LoRa.write(scrambled_message);
    LoRa.write(scrambled_message);
    
    return LoRa.endPacket();
}