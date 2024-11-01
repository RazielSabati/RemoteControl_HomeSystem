#include "DisplayMenu.h"

DisplayMenu::DisplayMenu(){}

void DisplayMenu::setupScreen() {
    this->display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // אתחול display באמצעות רשימת אתחול
    this->currentIndex = 0;
    this->isManualControl = false;

    // אתחול התצוגה ובדיקת הצלחה
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        while (true); // עצירת התוכנית אם אתחול נכשל
    }
    display.clearDisplay();

    renderBottomPanel();
    displayBatteryAndTime(100, "12:00");
}



void DisplayMenu::renderBottomPanel() {
    // ניקוי החלק השמאלי של המסך
    display.fillRect(0, 16, SCREEN_WIDTH, SCREEN_HEIGHT - 16, BLACK); // מחיקת החלק התחתון
    display.setCursor(0, 16); // קביעת המיקום להתחלת ההדפסה בחצי השמאלי
    display.setTextSize(1.8); // קביעת גודל טקסט
    display.setTextColor(WHITE);
    
    int menuType = isManualControl ? 1 : 0;

    for (int i = 0; i < numItems; i++) {
        if (i == currentIndex) 
            display.print("> ");
        else 
            display.print("  ");
        display.println(namesMatrix[menuType][i]);
    }

    // עדכון המסך עם התוכן החדש
    display.display();
}

// פונקציה להצגת אחוז הסוללה והשעה בחלק הימני העליון של המסך
void DisplayMenu::displayBatteryAndTime(int batteryPercentage, String time) {

    display.fillRect(53, 0,10, 16, BLACK); // מחיקת החצי העליון של החלק הימני

    display.setCursor(53, 0);
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.print("battery:");
    display.print(batteryPercentage);
    display.println("%");


    display.setCursor(53, 9);
    display.print("Time: ");
    display.println(time);

    display.display(); // עדכון התצוגה
}

void DisplayMenu::displayConfirmationMessage(String confirmationMessage) {
    // הצגת ההודעה
    display.fillRect(0, 0, SCREEN_WIDTH, 16, BLACK);
    display.setCursor(0,0);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print(confirmationMessage);
    display.display();
    
    // השהייה של 2.5 שניות
    delay(2500);
    
    // מחיקת האזור
    display.fillRect(0, 0, SCREEN_WIDTH, 16, BLACK);
    display.display();
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

const char* DisplayMenu::getData(int externalIndex   , int internalIndex) {
  return namesMatrix[externalIndex][internalIndex];
}

const int DisplayMenu::getCurrentIndex() {
  return this->currentIndex;
}

void DisplayMenu::enterManualControl() {
  isManualControl = true;
  currentIndex = 0; // start from the first item in the manual control menu
  renderBottomPanel();
}

bool DisplayMenu::isInManualControl() {
  return isManualControl;
}

void DisplayMenu::moveBackToMenu() {
  isManualControl = false; // back to the main menu
  currentIndex = 0;        // start from the first item in the main menu
  renderBottomPanel();
}
