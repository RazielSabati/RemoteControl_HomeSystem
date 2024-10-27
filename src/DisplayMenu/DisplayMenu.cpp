#include "DisplayMenu.h"

DisplayMenu::DisplayMenu() {
  this->currentIndex = 0;
  this->isManualControl = false;  //  start from the main menu
}

void DisplayMenu::display() {
  Serial.println("---------------");
  int menuType = isManualControl ? 1 : 0;
  
  for (int i = 0; i < numItems; i++) {
    if (i == currentIndex) {
      Serial.print("> ");
    } else {
      Serial.print("  ");
    }
    Serial.println(namesMatrix[menuType][i]);
  }
  Serial.println("---------------");
}

void DisplayMenu::moveUp() {
  currentIndex--;
  if (currentIndex < 0) {
    currentIndex = numItems - 1;
  }
  display();
}

void DisplayMenu::moveDown() {
  currentIndex++;
  if (currentIndex >= numItems) {
    currentIndex = 0;
  }
  display();
}

const char* DisplayMenu::getCurrentSelection() {
  int menuType = isManualControl ? 1 : 0;
  return namesMatrix[menuType][currentIndex];
}

const char* DisplayMenu::getData(int externalIndex   , int internalIndex) {
  return namesMatrix[externalIndex][internalIndex];
}

const int DisplayMenu::getCurrentIndex() {
  return this->currentIndex;
}

void DisplayMenu::enterManualControl() {
  isManualControl = true;
  currentIndex = 0; // start from the first item in the manual control menu
  display();
}

bool DisplayMenu::isInManualControl() {
  return isManualControl;
}

void DisplayMenu::moveBackToMenu() {
  isManualControl = false; // back to the main menu
  currentIndex = 0;        // start from the first item in the main menu
  display();
}
