#include "config.h"

TTGOClass *ttgo;
TFT_eSPI *tft;
AXP20X_Class *power;

//Si on doit couper l'ecrant, le bouton (irq = interupt)
bool irq = false;
//Quel mode somme nous (0 = Batterie, 1 = Clock)
unsigned int screenMode = 0;

//Simplification
#define boutonPin AXP202_INT

//Pour l'heure
uint32_t targetTime = 0;       // for next 1 second timeout
byte omm = 99;
boolean initial = 1;
byte xcolon = 0;
unsigned int colour = 0;
static uint8_t conv2d(const char *p){
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}
uint8_t hh = conv2d(__TIME__), mm = conv2d(__TIME__ + 3), ss = conv2d(__TIME__ + 6); // Get H, M, S from compile time


void setup() {
  Serial.begin(115200);
  //Récuperation des instance de la montre
  ttgo = TTGOClass::getWatch();
  //On initialise le materiel
  ttgo->begin();
  //On démarre la lumiere de l'ecrant
  ttgo->openBL();

  //Simplification
  tft = ttgo->tft;
  power = ttgo->power;
  
  //On met en route le moniteur de puissance
  power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);

  //On definie le bouton comme entrée
  pinMode(boutonPin, INPUT_PULLUP);
  //Si le bouton est presser alors on passe la variable irq a True
  attachInterrupt(boutonPin, [] {irq = true;}, FALLING);

  
  // Must be enabled first, and then clear the interrupt status,
  // otherwise abnormal
  power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
  //  Clear interrupt status
  power->clearIRQ();
 
  //Preparation de l'ecran
  //On met l'ecrant en noir
  tft->fillScreen(TFT_BLACK); 
  Serial.println("START");
}

void displayBatterie(){
  //On met la taille de l'ecriture a 2
  tft->setTextFont(2);
  //On choisie la couleur du texte a vert et le fond en noir
  tft->setTextColor(TFT_GREEN, TFT_BLACK);
  
  tft->fillRect(0, 0, 210, 130, TFT_BLACK);
  tft->setCursor(0, 0);

  tft->print("VBUS STATUS: ");
  // You can use isVBUSPlug to check whether the USB connection is normal
  if (power->isVBUSPlug()) {
    tft->println("CONNECT");

    // Get USB voltage
    tft->print("VBUS Voltage:");
    tft->print(power->getVbusVoltage());
    tft->println(" mV");

    // Get USB current
    tft->print("VBUS Current: ");
    tft->print(power->getVbusCurrent());
    tft->println(" mA");

  } else {

    tft->setTextColor(TFT_RED, TFT_BLACK);
    tft->println("DISCONNECT");
    tft->setTextColor(TFT_GREEN, TFT_BLACK);
  }

  tft->println();

  tft->print("BATTERY STATUS: ");
  // You can use isBatteryConnect() to check whether the battery is connected properly
  if (power->isBatteryConnect()) {
    tft->println("CONNECT");

    // Get battery voltage
    tft->print("BAT Voltage:");
    tft->print(power->getBattVoltage());
    tft->println(" mV");

    // To display the charging status, you must first discharge the battery,
    // and it is impossible to read the full charge when it is fully charged
    if (power->isChargeing()) {
      tft->print("Charge:");
      tft->print(power->getBattChargeCurrent());
      tft->println(" mA");
    } else {
      // Show current consumption
      tft->print("Discharge:");
      tft->print(power->getBattDischargeCurrent());
      tft->println(" mA");
      tft->print("Per: ");
      tft->print(power->getBattPercentage());
      tft->println(" %");

    }
  } else {
    tft->setTextColor(TFT_RED, TFT_BLACK);
    tft->println("DISCONNECT");
    tft->setTextColor(TFT_GREEN, TFT_BLACK);
  }
}

void displayClock(){
    if (targetTime < millis()) {
        targetTime = millis() + 1000;
        ss++;              // Advance second
        if (ss == 60) {
            ss = 0;
            omm = mm;
            mm++;            // Advance minute
            if (mm > 59) {
                mm = 0;
                hh++;          // Advance hour
                if (hh > 23) {
                    hh = 0;
                }
            }
        }
        if (ss == 0 || initial) {
            initial = 0;
            ttgo->tft->setTextColor(TFT_GREEN, TFT_BLACK);
            ttgo->tft->setCursor (8, 52);
            ttgo->tft->print(__DATE__); // This uses the standard ADAFruit small font

            ttgo->tft->setTextColor(TFT_BLUE, TFT_BLACK);
            ttgo->tft->drawCentreString("It is windy", 120, 48, 2); // Next size up font 2

            //ttgo->tft->setTextColor(0xF81F, TFT_BLACK); // Pink
            //ttgo->tft->drawCentreString("12.34",80,100,6); // Large font 6 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 . : a p m
        }

        // Update digital time
        byte xpos = 6;
        byte ypos = 0;
        if (omm != mm) { // Only redraw every minute to minimise flicker
            // Uncomment ONE of the next 2 lines, using the ghost image demonstrates text overlay as time is drawn over it
            ttgo->tft->setTextColor(0x39C4, TFT_BLACK);  // Leave a 7 segment ghost image, comment out next line!
            //ttgo->tft->setTextColor(TFT_BLACK, TFT_BLACK); // Set font colour to black to wipe image
            // Font 7 is to show a pseudo 7 segment display.
            // Font 7 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 0 : .
            ttgo->tft->drawString("88:88", xpos, ypos, 7); // Overwrite the text to clear it
            ttgo->tft->setTextColor(0xFBE0, TFT_BLACK); // Orange
            omm = mm;

            if (hh < 10) xpos += ttgo->tft->drawChar('0', xpos, ypos, 7);
            xpos += ttgo->tft->drawNumber(hh, xpos, ypos, 7);
            xcolon = xpos;
            xpos += ttgo->tft->drawChar(':', xpos, ypos, 7);
            if (mm < 10) xpos += ttgo->tft->drawChar('0', xpos, ypos, 7);
            ttgo->tft->drawNumber(mm, xpos, ypos, 7);
        }

        if (ss % 2) { // Flash the colon
            ttgo->tft->setTextColor(0x39C4, TFT_BLACK);
            xpos += ttgo->tft->drawChar(':', xcolon, ypos, 7);
            ttgo->tft->setTextColor(0xFBE0, TFT_BLACK);
        } else {
            ttgo->tft->drawChar(':', xcolon, ypos, 7);
            colour = random(0xFFFF);
            // Erase the old text with a rectangle, the disadvantage of this method is increased display flicker
            ttgo->tft->fillRect (0, 64, 160, 20, TFT_BLACK);
            ttgo->tft->setTextColor(colour);
            ttgo->tft->drawRightString("Colour", 75, 64, 4); // Right justified string drawing to x position 75
            String scolour = String(colour, HEX);
            scolour.toUpperCase();
            char buffer[20];
            scolour.toCharArray(buffer, 20);
            ttgo->tft->drawString(buffer, 82, 64, 4);
        }
    }
}

void loop(){
  Serial.println(irq);
  
  if (screenMode == 0){
    //on affiche le niveau de batterie
    displayBatterie();
  }
  if (screenMode == 1){
    displayClock();  
  }

  //Si on appuis sur l'ecran
  int16_t x, y;
  if (ttgo->getTouch(x, y)) {
    Serial.println("Change Mode");
    if (screenMode == 0){
      //On met l'ecrant en noir
      tft->fillScreen(TFT_BLACK);
      screenMode = 1;
    }else{
      if (screenMode == 1){
        //On met l'ecrant en noir
        tft->fillScreen(TFT_BLACK);
        screenMode = 0;
      }
    }
    delay(100);
  }
    
  if (irq) {
    irq = false;
    power->readIRQ();
    if (power->isPEKShortPressIRQ()) {
      
      irq = false;
      Serial.println("PowerOff");
      // Set screen and touch to sleep mode
      ttgo->displaySleep();
      ttgo->powerOff();

      // TOUCH SCREEN  Wakeup source
      esp_sleep_enable_ext1_wakeup(GPIO_SEL_38, ESP_EXT1_WAKEUP_ALL_LOW);
      // PEK KEY  Wakeup source
      // esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
      esp_deep_sleep_start();
    }
    power->clearIRQ();
  }
  
  delay(100);
}
