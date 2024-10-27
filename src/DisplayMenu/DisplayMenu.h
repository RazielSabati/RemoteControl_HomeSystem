#ifndef DISPLAYMENU_H
#define DISPLAYMENU_H

#include <Arduino.h>

class DisplayMenu {
  private:
    // matrix of menu items
    const char* namesMatrix[2][6] = {
      { "Test scenario", "Read battery percentage", "Turn off system (S.M)", "Scenario 1", "Scenario 2", "Manual control" },
      { "Turn on LED", "Turn off LED", "Turn on voice", "Turn off voice", "Turn on heat", "Turn off heat" }
    };
    
    int numItems = 6;
    int currentIndex;
    bool isManualControl;  // true if we are in manual control menu

  public:
    DisplayMenu(); // Constructor

    void display();
    void moveUp();
    void moveDown();
    const char* getCurrentSelection();
    const char* getData(int externalIndex , int internalIndex);
    const int getCurrentIndex();
    void enterManualControl(); 
    bool isInManualControl();   
    void moveBackToMenu();      
};

#endif // DISPLAYMENU_H
