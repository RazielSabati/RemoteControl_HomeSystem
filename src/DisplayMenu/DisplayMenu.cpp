#include "DisplayMenu.h"

DisplayMenu::DisplayMenu() {}

void DisplayMenu::setupScreen() {
    this->display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Initialize display with initialization list
    this->currentIndex = 0;
    this->isManualControl = false;

    // Initialize the display and check for success
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        while (true); // Halt the program if initialization fails
    }
    display.clearDisplay();

    renderBottomPanel();
    // displayBatteryAndTime(100, "12:00");
}

void DisplayMenu::renderBottomPanel() {
    // Clear the bottom part of the screen
    display.fillRect(0, 16, SCREEN_WIDTH, SCREEN_HEIGHT - 16, BLACK); // Clear the bottom part
    display.setCursor(0, 16); // Set the cursor position to start printing on the left half
    display.setTextSize(1.8); // Set text size
    display.setTextColor(WHITE);
    
    int menuType = isManualControl ? 1 : 0;

    for (int i = 0; i < numItems; i++) {
        if (i == currentIndex) 
            display.print("> ");
        else 
            display.print("  ");
        display.println(namesMatrix[menuType][i]);
    }

    // Update the screen with the new content
    display.display();
}

// Function to display battery percentage and time in the top right corner of the screen
void DisplayMenu::displayBatteryAndTime(int batteryPercentage, String time) {
    display.fillRect(53, 0, 10, 16, BLACK); // Clear the top half of the right part

    display.setCursor(53, 0);
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.print("battery:");
    display.print(batteryPercentage);
    display.println("%");

    display.setCursor(53, 9);
    display.print("Time: ");
    display.println(time);

    display.display(); // Update the display
}

void DisplayMenu::displayConfirmationMessage(String confirmationMessage, int confirm) {
    // Display the message
    display.fillRect(0, 0, SCREEN_WIDTH, 16, BLACK);
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print(confirmationMessage);
    display.display();
    
    if (confirm == 0) {
        // Delay for 1.5 seconds
        delay(1500);
        display.fillRect(0, 0, SCREEN_WIDTH, 16, BLACK);
        display.display();
        return;
    }
}

void DisplayMenu::moveUp() {
    currentIndex--;
    if (currentIndex < 0) {
        currentIndex = numItems - 1;
    }
    renderBottomPanel();
}

void DisplayMenu::moveDown() {
    currentIndex++;
    if (currentIndex >= numItems) {
        currentIndex = 0;
    }
    renderBottomPanel();
}

const char* DisplayMenu::getCurrentSelection() {
    int menuType = isManualControl ? 1 : 0;
    return namesMatrix[menuType][currentIndex];
}

const char* DisplayMenu::getData(int externalIndex, int internalIndex) {
    return namesMatrix[externalIndex][internalIndex];
}

const int DisplayMenu::getCurrentIndex() {
    return this->currentIndex;
}

void DisplayMenu::enterManualControl() {
    isManualControl = true;
    currentIndex = 0; // Start from the first item in the manual control menu
    renderBottomPanel();
}

bool DisplayMenu::isInManualControl() {
    return isManualControl;
}

void DisplayMenu::moveBackToMenu() {
    isManualControl = false; // Back to the main menu
    currentIndex = 0;        // Start from the first item in the main menu
    renderBottomPanel();
}