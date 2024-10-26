#include "DisplayMenu.h"

DisplayMenu::DisplayMenu() {
  this->currentIndex = 0;
  this->isManualControl = false;  // מתחילים בתפריט הראשי
}

void DisplayMenu::display() {
  Serial.println("---------------");
  if (isManualControl) {
    for (int i = 0; i < numItems; i++) {
      if (i == currentIndex) {
        Serial.print("> ");
      } else {
        Serial.print("  ");
      }
      Serial.println(manualControlItems[i]);
    }
  } else {
    for (int i = 0; i < numItems; i++) {
      if (i == currentIndex) {
        Serial.print("> ");
      } else {
        Serial.print("  ");
      }
      Serial.println(items[i]);
    }
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
  if (isManualControl) {
    return manualControlItems[currentIndex];
  }
  return items[currentIndex];
}

void DisplayMenu::enterManualControl() {
  isManualControl = true;
  currentIndex = 0; // מתחילים מהפריט הראשון בתפריט הידני
  display();
}

bool DisplayMenu::isInManualControl() {
  return isManualControl;
}

void DisplayMenu::moveBackTOMenue() {
  isManualControl = false; // חוזרים לתפריט הראשי
  currentIndex = 0;        // אפשר להתחיל שוב מהפריט הראשון בתפריט הראשי
  display();
}