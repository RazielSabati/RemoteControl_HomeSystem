#include "./communication/bluetooth_wrapper.h"
#include "./communication/packet_handler.h"
#include "./communication/lora_wrapper.h"

// communication_protocol_interface_t g_bluetooth_communicator;
extern bt_settings_t g_bt;
extern Lora_settings_t g_lora;

// // Declaration for the listenForAcknowledgment function
// void listenForAcknowledgment(void *parameter);

// HomeCommunication loraCommunication;
// Buttons buttons(32, 33, 25, 27);
// DisplayMenu menu;

// bool isWaitingForAck = false; // Global variable
// unsigned long waitStartTime = 0;
// const unsigned long ACK_TIMEOUT = 30000;       // 20 second timeout
// TaskHandle_t ackTaskHandle = NULL;             // Handle for the acknowledgment task

// // intialize the last call time for send_keep_alive function
// unsigned long lastCallToA = 0;
// unsigned long A_CALL_INTERVAL = 20000;  // 20 שניות

void setup()
{
    Serial.begin(9600);
    SPI.begin();

    if (!bluetooth__setup(&g_bt))
    {
        digitalWrite(STATUS_LED_RED_PIN, HIGH);
        delay(300);
        digitalWrite(STATUS_LED_RED_PIN, LOW);
        delay(300);
    }

    if (!setup_Lora_module(&g_lora))
    {
        Serial.println(F("Failed to initialize LoRa packet parser! Halting."));
        while (1)
        {
            delay(1000);
        }
    }
}

void loop()
{
    bluetooth__poll_and_process_packets(&g_bt, &process_incoming_packet);
    lora_packet_parser__poll();
    // communication_protocol__run_main_logic(&g_bluetooth_communicator, &packet_handler__process);
}
//     unsigned long currentMillis = millis();

//     // Check for timeout on waiting ACK
//     if (isWaitingForAck && currentMillis - waitStartTime >= ACK_TIMEOUT) {
//         Serial.println(F("Acknowledgment timeout"));
//         menu.displayConfirmationMessage("Timeout,try again!", 0);
//         isWaitingForAck = false;
//         loraCommunication.rollingCode++; // Increment the rolling code

//         // Delete the task if it was created (to free the core)
//         if (ackTaskHandle != NULL) {
//             vTaskDelete(ackTaskHandle);
//             ackTaskHandle = NULL;
//         }
//     }

//         // Check if it's time to send a keep alive message
//     if ( !isWaitingForAck && currentMillis - lastCallToA >= A_CALL_INTERVAL) {
//         menu.displayConfirmationMessage("Keep alive sent", 1);
//         if ( !loraCommunication.send_keep_alive() )  // send_keep_alive failed
//             menu.displayConfirmationMessage("Keep alive failed", 1);
//         lastCallToA = currentMillis;  // update the next time
//         menu.displayConfirmationMessage("Keep alive done", 0);

//         A_CALL_INTERVAL = random(18000, 30000);  // update the interval
//     }

//     // Button handling with debounce
//     static unsigned long lastButtonPress = 0;
//     const unsigned long DEBOUNCE_DELAY = 200;

//     if (currentMillis - lastButtonPress >= DEBOUNCE_DELAY) {
//         if (buttons.isUpPressed()) {
//             menu.moveUp();
//             lastButtonPress = currentMillis;
//         }

//         if (buttons.isDownPressed()) {
//             menu.moveDown();
//             lastButtonPress = currentMillis;
//         }

//         if (buttons.isBackPressed() && menu.isInManualControl()) {
//             menu.moveBackToMenu();
//             lastButtonPress = currentMillis;
//         }

//         if (buttons.isSelectPressed()) {
//             if (strcmp(menu.getCurrentSelection(), "manual control") == 0) {
//                 menu.enterManualControl();
//             }
//             else if (!isWaitingForAck) {

//                 int menuType = menu.isInManualControl() ? 1 : 0;
//                 int actionIndex = menu.getCurrentIndex();

//                 menu.displayConfirmationMessage("send " + (String)(menu.getData(menuType, actionIndex)) , 1);

//                 // Send the message and start waiting for an ACK
//                 if (loraCommunication.sendMessage(menu, menuType, actionIndex)) {

//                     menu.displayConfirmationMessage(loraCommunication.getLastRequest() + " sent", 1);
//                     isWaitingForAck = true;
//                     waitStartTime = currentMillis;

//                     // Create the acknowledgment task only when waiting for an ACK
//                     xTaskCreatePinnedToCore(
//                         listenForAcknowledgment, // Task function
//                         "ListenForAck",          // Task name
//                         10000,                   // Stack size
//                         NULL,                    // Parameters
//                         1,                       // Priority
//                         &ackTaskHandle,          // Task handle
//                         0                        // Core to pin the task
//                     );
//                 }
//                 else
//                     menu.displayConfirmationMessage(loraCommunication.getLastRequest() + " failed", 0);

//             }
//             lastButtonPress = currentMillis;
//         }
//     }
// }

// // Task to listen for acknowledgment messages (runs on core 0)
// void listenForAcknowledgment(void *parameter) {
//     while (isWaitingForAck) {  // Run only if still waiting for ACK
//         if (loraCommunication.checkForAcknowledgment(isWaitingForAck, menu)) {
//             Serial.println(F("Acknowledgment received in task"));
//             isWaitingForAck = false;

//             // Delete the task once acknowledgment is received
//             if (ackTaskHandle != NULL) {
//                 vTaskDelete(ackTaskHandle);
//                 ackTaskHandle = NULL;
//             }
//             break; // Exit the loop after acknowledgment is handled
//         }
//         vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to prevent overloading
//     }
//     vTaskDelete(NULL); // Delete the current task
// }