#ifndef DISPLAYMENU_H
#define DISPLAYMENU_H

#include <Arduino.h>

class DisplayMenu {
  private:
       // תפריט ראשי
    const char* items[6] = {
      "Test scenario",
      "Read battery percentage",
      "Turn off system (S.M)",
      "Scenario 1",
      "Scenario 2",
      "Manual control"
    };

    const char* manualControlItems[6] = {
      "Turn on LED",
      "Turn off LED",
      "Turn on voice",
      "Turn off voice",
      "Turn on heat",
      "Turn off heat"
    };
    
    int numItems  = 6;
    int currentIndex;
    bool isManualControl;  // משתנה למעקב אחרי איזה תפריט פעיל

  public:
   DisplayMenu(); // הגדרת הבנאי

    void display();
    void moveUp();
    void moveDown();
    const char* getCurrentSelection();
    void enterManualControl(); // מעבר לתפריט הידני
    bool isInManualControl();   // בודק האם אנחנו בתפריט הידני
    void moveBackTOMenue();     // חזרה לתפריט הראשי

};

#endif // DISPLAYMENU_H