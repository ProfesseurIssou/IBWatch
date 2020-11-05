#include "config.h"

TTGOClass *ttgo;
TFT_eSPI *tft;
AXP20X_Class *power;

void setup() {
  //...
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
  
  //Preparation de l'ecran
  //On met l'ecrant en noir
  tft->fillScreen(TFT_BLACK); 
  //...
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




void loop(){
  //...
  displayBatterie();
  delay(100);
  //...
}
