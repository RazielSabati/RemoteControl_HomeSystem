#ifndef DISPLAYMENU_H
#define DISPLAYMENU_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1  // Using OLED without RESET pin
#define MAX_MESSAGES 4 // Maximum number of messages to display

class DisplayMenu {
  private:
    // Matrix of menu items
    const char* namesMatrix[2][6] = {
      { "test scenario", "read battery %", "turn off system", "scenario 1", "scenario 2", "manual control" },
      { "turn on led", "turn off led", "turn on voice", "turn off voice", "turn on heat", "turn off heat" }
    };

    Adafruit_SSD1306 display;
    
    int numItems = 6;
    int currentIndex;
    bool isManualControl;  // True if we are in manual control menu

  public:
    DisplayMenu(); // Constructor
    void setupScreen();

    void renderBottomPanel();
    void displayBatteryAndTime(int batteryPercentage, String time); // Function to display battery percentage and time
    void displayConfirmationMessage(String confirmationMessage, int confirm); // Function to display confirmation message
    void moveUp();
    void moveDown();
    const char* getCurrentSelection();
    const char* getData(int externalIndex, int internalIndex);
    const int getCurrentIndex();
    void enterManualControl(); 
    bool isInManualControl();   
    void moveBackToMenu();      
};

#endif // DISPLAYMENU_H