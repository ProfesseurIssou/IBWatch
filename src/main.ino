#include "config.h"

TTGOClass *ttgo;
TFT_eSPI *tft;
AXP20X_Class *power;

//Si on doit couper l'ecrant
bool irq = false;

void setup()
{
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

    //On met l'ecrant en noir
    tft->fillScreen(TFT_BLACK);
    //On met la taille de l'ecriture a 2
    tft->setTextFont(2);
    //On choisie la couleur du texte a vert et le fond en noir
    tft->setTextColor(TFT_GREEN, TFT_BLACK);

    //On definie le bouton comme entrée
    pinMode(AXP202_INT, INPUT_PULLUP);
    //Si le bouton est presser alors on passe la variable irq a True
    attachInterrupt(AXP202_INT, [] {
        irq = true;
    }, FALLING);
    //!Clear IRQ unprocessed  first
    power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
    power->clearIRQ();
}

void displayBatterie(){
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

void loop()
{
    //on affiche le niveau de batterie
    displayBatterie();

    if (irq) {
        irq = false;
        power->readIRQ();
        if (power->isPEKShortPressIRQ()) {
            // Clean power chip irq status
            power->clearIRQ();

            // Set  touchscreen sleep
            ttgo->displaySleep();

            /*
            In TWatch2019/ Twatch2020V1, touch reset is not connected to ESP32,
            so it cannot be used. Set the touch to sleep,
            otherwise it will not be able to wake up.
            Only by turning off the power and powering on the touch again will the touch be working mode
            // ttgo->displayOff();
            */

            ttgo->powerOff();

            //Set all channel power off
            power->setPowerOutPut(AXP202_LDO3, false);
            power->setPowerOutPut(AXP202_LDO4, false);
            power->setPowerOutPut(AXP202_LDO2, false);
            power->setPowerOutPut(AXP202_EXTEN, false);
            power->setPowerOutPut(AXP202_DCDC2, false);

            // TOUCH SCREEN  Wakeup source
            esp_sleep_enable_ext1_wakeup(GPIO_SEL_38, ESP_EXT1_WAKEUP_ALL_LOW);
            // PEK KEY  Wakeup source
            // esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
            esp_deep_sleep_start();
        }
        power->clearIRQ();
        delay(1000);
    }
}
