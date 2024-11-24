#ifndef HOME_COMMUNICATION_H
#define HOME_COMMUNICATION_H

#include <SPI.h>
#include <LoRa.h>
#include <Arduino.h>
#include "../DisplayMenu/DisplayMenu.h"
#include <AESLib.h>

class HomeCommunication {
private:
    static const uint8_t LORA_SS_PIN = 5;
    static const uint8_t LORA_RST_PIN = 14;
    static const uint8_t LORA_DIO0_PIN = 2;

    String last_request; // Stores the last request sent
    uint8_t last_request_code; // Stores the last request code
    uint8_t message; // Message to be sent
    int randomNumber; // Random number for encryption

    byte key[16] = {0x2b, 0x7e, 0x15, 0x16, 0xae, 0x2a, 0xd4, 0xa6, 0x8a, 0xf7, 0x97, 0x75, 0x40, 0x8e, 0x6a, 0x12}; // AES encryption key

    uint8_t buffer[32]; // Buffer for incoming messages

    byte iv[16]; // Initialization vector for decryption
    byte iv_for_en[16]; // Initialization vector for encryption
    uint8_t plaintext[16]; // Original message
    uint8_t encrypted[16]; // Encrypted message
    uint8_t decrypted[16]; // Decrypted message

    AESLib aesLib; // AES library instance

public:
    HomeCommunication(); // Constructor
    bool setupCommunication(); // Sets up the communication
    bool sendMessage(DisplayMenu& menu, int menuType, int actionIndex); // Sends a message
    bool checkForAcknowledgment(bool& isWaitingForAck, DisplayMenu& menu); // Checks for acknowledgment
    bool send_keep_alive(); // Sends a keep-alive message
    void generateRandomIV(); // Generates a random initialization vector
    String getLastRequest(); // Gets the last request sent
    uint32_t rollingCode; // Rolling code for encryption
};

#endif // HOME_COMMUNICATION_H