#include "Buttons.h"


void Buttons::setupButtons() {
  // Set the button as input with pull-up resistor    
  pinMode(buttonUp, INPUT_PULLUP); 
  pinMode(buttonDown, INPUT_PULLUP);
  pinMode(buttonSelect, INPUT_PULLUP);
  pinMode(buttonBack, INPUT_PULLUP);
}

bool Buttons::isUpPressed() {
  return digitalRead(buttonUp) == LOW;
}

bool Buttons::isDownPressed() {
  return digitalRead(buttonDown) == LOW;
}

bool Buttons::isSelectPressed() {
  return digitalRead(buttonSelect) == LOW;
}

bool Buttons::isBackPressed() {
  return digitalRead(buttonBack) == LOW;
}
