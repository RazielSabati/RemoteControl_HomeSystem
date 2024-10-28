#include "./communication/communication.h"
#include "./Buttons/Buttons.h"
#include "./DisplayMenu/DisplayMenu.h"

HomeCommunication loraCommunication;
Buttons buttons(32, 33, 25, 27);
DisplayMenu menu;

bool isWaitingForAck = false;  // is waiting for acknowledgment
unsigned long waitStartTime = 0;
const unsigned long ackTimeout = 10000;  // 10 seconds wait for acknowledgment

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
        delay(200);  // avoid bouncing
    }

    if (buttons.isDownPressed()) {
        menu.moveDown();
        delay(200);  // avoid bouncing
    }

    if (buttons.isBackPressed() && menu.isInManualControl() ) {
        menu.moveBackToMenu();
        delay(200);  // avoid bouncing
    }

    if (buttons.isSelectPressed() && strcmp(menu.getCurrentSelection(), "Manual control") == 0) {
        menu.enterManualControl();
        delay(200);  // avoid bouncing
        return;
    }

    // Check if the select button is pressed and send the selected message
    if (!isWaitingForAck && buttons.isSelectPressed()) {
        int menuType = menu.isInManualControl() ? 2 : 1;  // 1 for main menu, 2 for manual control menu
        int actionIndex = menu.getCurrentIndex();          // get the selected action index
        int commandCode = menuType * 10 + actionIndex;     // create a command code

        loraCommunication.sendMessage(String(commandCode) , menu , menuType -1 , actionIndex ); // send the selected message
        isWaitingForAck = true;
        waitStartTime = millis();
        delay(500);  // avoid bouncing
    }

}