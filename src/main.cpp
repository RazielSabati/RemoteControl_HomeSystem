#include "./communication/communication.h"
#include "./Buttons/Buttons.h"
#include "./DisplayMenu/DisplayMenu.h"

HomeCommunication loraCommunication;
Buttons buttons(32, 33, 25, 27);
DisplayMenu menu;

bool isWaitingForAck = false;  // האם מחכים לאישור ממערכת החוץ
unsigned long waitStartTime = 0;
const unsigned long ackTimeout = 10000;  // 10 שניות המתנה לאישור

void setup() {
    // Initialize home system configuration
    Serial.begin(9600);
    while (!Serial);          // Wait until Serial is ready

    loraCommunication.setupCommunication();
    buttons.setupButtons();
    menu.display();


    Serial.println("Home system ready to send messages.");
}

void loop() {
    if (isWaitingForAck) {
        loraCommunication.checkForAcknowledgment(isWaitingForAck);
    }

    if (isWaitingForAck && (millis() - waitStartTime > ackTimeout)) {
        Serial.println("Timeout: No acknowledgment received.");
        isWaitingForAck = false;
    }

    if (buttons.isUpPressed()) {
        menu.moveUp();
        delay(200);  // מניעת ריצוד
    }

    if (buttons.isDownPressed()) {
        menu.moveDown();
        delay(200);  // מניעת ריצוד
    }

    if (buttons.isBackPressed() && menu.isInManualControl() ) {
        menu.moveBackTOMenue();
        delay(200);  // מניעת ריצוד
    }

    if (buttons.isSelectPressed() && strcmp(menu.getCurrentSelection(), "Manual control") == 0) {
        menu.enterManualControl();
        delay(200);  // מניעת ריצוד
        return;
    }

    // Check if the select button is pressed and send the selected message
    if (!isWaitingForAck && buttons.isSelectPressed()) {
        loraCommunication.sendMessage(menu.getCurrentSelection());
        isWaitingForAck = true;
        waitStartTime = millis();
        delay(500);  // מניעת ריצוד
    }

}