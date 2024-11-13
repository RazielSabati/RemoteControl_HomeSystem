#ifndef HOME_COMMUNICATION_H
#define HOME_COMMUNICATION_H

#include <SPI.h>
#include <LoRa.h>
#include <Arduino.h>
#include "../DisplayMenu/DisplayMenu.h"

class HomeCommunication {
private:
    String last_request;
    uint8_t last_request_code;
    int randomNumber;

    static const uint8_t LORA_SS_PIN = 5;
    static const uint8_t LORA_RST_PIN = 14;
    static const uint8_t LORA_DIO0_PIN = 2;

public:
    HomeCommunication();
    bool setupCommunication();
    bool checkForAcknowledgment(bool& isWaitingForAck, DisplayMenu& menu);
    bool sendMessage(DisplayMenu& menu, int menuType, int actionIndex);
    uint8_t lfsrScramble(uint8_t unscrambled);
    uint8_t lfsrUnscramble(uint8_t scrambled); 
    String getLastRequest();
    bool send_keep_alive();

};

#endif

