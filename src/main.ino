#include "config.h"

TTGOClass *ttgo;
TFT_eSPI *tft;
AXP20X_Class *power;

//Si on doit couper l'ecrant, le bouton (irq = interupt)
bool irq = false;

//Quel mode somme nous (0 = Batterie, 1 = Clock)
unsigned int screenMode = 0;

//Si on allume l'ecran
bool screenDisplay = true;

//Simplification
#define boutonPin AXP202_INT

/*Pour l'heure*/
uint32_t targetTime = 0;       // for next 1 second timeout
byte minuteCache = 99;         // Pour l'affichage de l'heure (evité l'effet stroboscope)
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
  //On démarre la lumiere de l'ecran
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

//Calculer l'heure
void timeCalc(){
  if (targetTime < millis()) {
    targetTime = millis() + 1000;
    ss++;              // Advance second
    if (ss == 60) {
      ss = 0;
      minuteCache = mm;
      mm++;            // Advance minute
      if (mm > 59) {
        mm = 0;
        hh++;          // Advance hour
        if (hh > 23) {
          hh = 0;
        }
      }
    }
  }
}

//Pour gérer l'allumage ou non de l'ecran
void screenDisplayer(){
  if (screenDisplay == true){
    ttgo->openBL();
  }else{
    ttgo->closeBL();
  }
}

void displayBatterie(){
  //On met la taille de l'ecriture a 2
  tft->setTextFont(2);
  //On choisie la couleur du texte a vert et le fond en noir
  tft->setTextColor(TFT_GREEN, TFT_BLACK);
  //On met le curseur en haut à gauche
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

//L'ecran de l'horloge
void clockScreen(){
  if (minuteCache != mm) { //Si les minute on changer
    Serial.println("reload hour");
    displayClock();
  }
}
void displayClock(){
  byte xpos = 6;
  byte ypos = 0;
  ttgo->tft->setTextColor(0x39C4, TFT_BLACK);         //On change la couleur du texte
  // Font 7 is to show a pseudo 7 segment display.
  // Font 7 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 0 : .
  ttgo->tft->drawString("88:88", xpos, ypos, 7);      //On affiche les segment pour l'ecriture de l'heure
  ttgo->tft->setTextColor(0xFBE0, TFT_BLACK);         // Orange
  minuteCache = mm;                                   //On remet le cache à jour

  //On verifie si on remplis la case vide des heure (un numero sur deux)
  if (hh < 10){
    xpos += ttgo->tft->drawChar('0', xpos, ypos, 7);
  }
  xpos += ttgo->tft->drawNumber(hh, xpos, ypos, 7);
  xpos += ttgo->tft->drawChar(':', xpos, ypos, 7);
  //On verifie si on remplis la case vide des minute (un numero sur deux)
  if (mm < 10){ 
    xpos += ttgo->tft->drawChar('0', xpos, ypos, 7);
  }
  ttgo->tft->drawNumber(mm, xpos, ypos, 7);
}

void loop(){
  //On verifie si l'ecran doit etre alumé ou non
  screenDisplayer();
  //Calcule de l'heure
  timeCalc();

  //Si l'ecran est allumé
  if (screenDisplay == true){
    if (screenMode == 0){
      //on affiche le niveau de batterie
      displayBatterie();
    }
    if (screenMode == 1){
      //on affiche l'heure
      clockScreen();  
    }

    //Si on appuis sur l'ecran
    int16_t x, y;
    if (ttgo->getTouch(x, y)) {
      Serial.println("Change Mode");
      if (screenMode == 0){//on passe du mode batterie à mode heure
        //On met l'ecran en noir
        tft->fillScreen(TFT_BLACK);
        screenMode = 1;
        displayClock();//on affiche l'heure
      }else{
        if (screenMode == 1){//on passe du mode heure à mode batterie
          //On met l'ecrant en noir
          tft->fillScreen(TFT_BLACK);
          screenMode = 0;
        }
      }
      delay(100);
    }
  }


  //bouton pressé
  if (irq) {
    //on remet la variable trigger à False
    irq = false;
    //on regarde le type d'interuption
    power->readIRQ();
    //si l'interuption est une pression rapide
    if (power->isPEKShortPressIRQ()) {
      //Si l'ecran est allumer alors on l'eteind
      if (screenDisplay == true){
        Serial.println("Turn off screen");
        screenDisplay = false;
      }else{
        //Si l'ecran est eteind alors on l'allume
        screenDisplay = true;
        Serial.println("Turn on screen");
      }
      
      
      // Set screen and touch to sleep mode
      //ttgo->displaySleep();
      //ttgo->powerOff();

      // TOUCH SCREEN  Wakeup source
      // esp_sleep_enable_ext1_wakeup(GPIO_SEL_38, ESP_EXT1_WAKEUP_ALL_LOW);
      // PEK KEY  Wakeup source
      //esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
      //esp_deep_sleep_start();
    }
    Serial.println(screenDisplay);
    power->clearIRQ();
  }
  
  delay(100);
}
