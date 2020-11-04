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
