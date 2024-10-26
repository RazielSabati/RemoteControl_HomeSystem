#ifndef Buttons_H
#define Buttons_H
#include <Arduino.h>

class Buttons {
  private:
    int buttonUp, buttonDown, buttonSelect,buttonBack;
    
  public:
    Buttons(int upPin, int downPin, int selectPin, int backPin) 
      : buttonUp(upPin), buttonDown(downPin), buttonSelect(selectPin), buttonBack(backPin) {}
    void setupButtons();  // Initialize the buttons
    bool isUpPressed();  // Check if the up button is pressed
    bool isDownPressed();  // Check if the down button is pressed
    bool isSelectPressed();  // Check if the select button is pressed
    bool isBackPressed();  // Check if the back button is pressed
};

#endif