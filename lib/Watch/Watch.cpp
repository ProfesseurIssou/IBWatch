#include "Watch.h"

Watch *ptrWatch;

//INIT/
//Contructeur
Watch::Watch(){
    //Variables
    //Programme
    ptrWatch = this;                                                            //Pointeur vers l'instance
    
    buttonPressed = false;                                                      //Par defaut
    alarmRing = false;                                                          //Par defaut
    irqShortPress = false;                                                      //Par defaut

    pinMode(RTC_PIN,INPUT_PULLUP);                                              //Pin du rtc comme entrée
    attachInterrupt(RTC_INT_PIN,[]{ptrWatch->SetAlarmRing(true);},FALLING);     //Si l'horloge arrive à une alarme alors la variable alarmRing == True
    pinMode(BUTTON_PIN,INPUT_PULLUP);                                           //Pin du bouton lateral comme entrée
    attachInterrupt(BUTTON_PIN,[]{ptrWatch->SetButtonPressed(true);},FALLING);  //Si le bouton est presser alors on passe la variable buttonPressed == True
    pinMode(BMA423_INT1, INPUT);                                                //Pin du comtpeur de pas en entrée
    attachInterrupt(BMA423_INT1,[]{ptrWatch->SetStepDetected(true);}, RISING);  //Si le compteur de pas detecte quelque chose alots on passe la variable stepDetected == True


    watch = TTGOClass::getWatch();                                              //Instance de la montre
    watch->begin();                                                             //Initialisation du materiel
    watch->motor_begin();                                                       //Demarrage moteur de vibration pin IO4

    screen = watch->tft;                                                        //Gestion de l'ecran
    power = watch->power;                                                       //Gestion de l'alimentation
    motor = watch->motor;                                                       //Gestion du moteur
    rtc = watch->rtc;                                                           //Gestion de l'horloge interne
    sensor = watch->bma;                                                        //Gestion de la detection des mouvement

    power->adc1Enable(AXP202_VBUS_VOL_ADC1|AXP202_VBUS_CUR_ADC1|AXP202_BATT_CUR_ADC1|AXP202_BATT_VOL_ADC1,true);//On met en route le moniteur de puissance
    power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ,true);                           //On met en place le systeme d'interuption grace au bouton
    power->clearIRQ();                                                          //On vide l'interuption

    // Accel parameter structure (BMA423 accelerometer (Gyroscope/Accelerometre))
    Acfg cfg;
    //Output data rate in Hz, Optional parameters:
    //   - BMA4_OUTPUT_DATA_RATE_0_78HZ
    //   - BMA4_OUTPUT_DATA_RATE_1_56HZ
    //   - BMA4_OUTPUT_DATA_RATE_3_12HZ
    //   - BMA4_OUTPUT_DATA_RATE_6_25HZ
    //   - BMA4_OUTPUT_DATA_RATE_12_5HZ
    //   - BMA4_OUTPUT_DATA_RATE_25HZ
    //   - BMA4_OUTPUT_DATA_RATE_50HZ
    //   - BMA4_OUTPUT_DATA_RATE_100HZ
    //   - BMA4_OUTPUT_DATA_RATE_200HZ
    //   - BMA4_OUTPUT_DATA_RATE_400HZ
    //   - BMA4_OUTPUT_DATA_RATE_800HZ
    //   - BMA4_OUTPUT_DATA_RATE_1600HZ
    cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    //G-range, Optional parameters:
    //   - BMA4_ACCEL_RANGE_2G
    //   - BMA4_ACCEL_RANGE_4G
    //   - BMA4_ACCEL_RANGE_8G
    //   - BMA4_ACCEL_RANGE_16G
    cfg.range = BMA4_ACCEL_RANGE_2G;
    //Bandwidth parameter, determines filter configuration, Optional parameters:
    //   - BMA4_ACCEL_OSR4_AVG1
    //   - BMA4_ACCEL_OSR2_AVG2
    //   - BMA4_ACCEL_NORMAL_AVG4
    //   - BMA4_ACCEL_CIC_AVG8
    //   - BMA4_ACCEL_RES_AVG16
    //   - BMA4_ACCEL_RES_AVG32
    //   - BMA4_ACCEL_RES_AVG64
    //   - BMA4_ACCEL_RES_AVG128
    cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    //Filter performance mode , Optional parameters:
    //   - BMA4_CIC_AVG_MODE
    //   - BMA4_CONTINUOUS_MODE
    cfg.perf_mode = BMA4_CONTINUOUS_MODE;
    //Configure the BMA423 accelerometer
    sensor->accelConfig(cfg);
    //Enable BMA423 accelerometer
    sensor->enableAccel();
    // You can also turn it off
    // sensor->disableAccel();
    sensor->enableFeature(BMA423_STEP_CNTR,true);                               //Activation du compteur de pas BMA423
    sensor->resetStepCounter();                                                 //On remet a 0 le compteur
    sensor->enableStepCountInterrupt();                                         //On active l'interruption du compteur de pas


    screen->setSwapBytes(1);                                                    //Convertion pour l'affichage des images (inversement des octets)
    screen->fillScreen(TFT_BLACK);                                              //On remplis l'ecran en noir
}
//Destructeur
Watch::~Watch(){
    //Variables
    //Programme
}
//Lance le deep sleep de la montre
void Watch::DeepSleep(bool prmWakeButton,bool prmWakeTouch){
    //Variables
    //PRogramme
    watch->displaySleep();                                                                      //Arret de l'affichage
    watch->powerOff();                                                                          //Arret du tactile

    //Set all channel power off
    power->setPowerOutPut(AXP202_LDO3, false);
    power->setPowerOutPut(AXP202_LDO4, false);
    power->setPowerOutPut(AXP202_LDO2, false);
    power->setPowerOutPut(AXP202_EXTEN, false);
    power->setPowerOutPut(AXP202_DCDC2, false);

    if(prmWakeButton){                                                                          //Si le reveil est avec le bouton
        esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);                         //On met le reveil sur le bouton
    }
    if(prmWakeTouch){                                                                           //Si le reveil est avec le tactile
        esp_sleep_enable_ext1_wakeup(GPIO_SEL_38, ESP_EXT1_WAKEUP_ALL_LOW);                         //On met le reveil sur le tactile
    }
    esp_deep_sleep_start();                                                                     //On lance le deepsleep
}
//####//


//GETTER SETTER//
//Si le bouton est pressé
bool Watch::ButtonPressed(){
    //Variables
    //Programme
    return buttonPressed;                                                       //Retourne l'etat du bouton
}
//Definition de l'etat du bouton
void Watch::SetButtonPressed(bool prmState){
    //Variables
    //Programme
    buttonPressed = prmState;                                                   //Definition de l'etat du bouton
}
//Si l'alarme sonne
bool Watch::AlarmRing(){
    //Variables
    //Programme
    return alarmRing;                                                           //Retourne l'etat de l'alarme
}
//Definition de l'etat de l'alarme
void Watch::SetAlarmRing(bool prmState){
    //Variables
    //Programme
    alarmRing = prmState;                                                       //Definition de l'etat de l'alarm
}
//Si des pas on été detecté
bool Watch::StepDetected(){
    //Variables
    //Programme
    return stepDetected;                                                        //Retourne l'etat de la detection des pas
}
//Definition de l'etat des pas on été detecté
void Watch::SetStepDetected(bool prmState){
    //Variables
    //Programme
    stepDetected = prmState;                                                    //Definition de l'etat de l'alarm
}
//Si le bouton a été pressé
bool Watch::GetIRQShortPress(){
    //Variables
    //Programme
    return irqShortPress;                                                               //On retourne la valeur de l'etat
}
//#############//

//WATCH FUNCTION//
//Gestion du retro eclairage (prmBrightness [0,255])
void Watch::SetBackLight(bool prmState,uint prmBrightness){
    //Variables
    //Programme
    if(prmState)watch->openBL();                                                //Si on allume
    if(!prmState)watch->closeBL();                                              //Si on eteint
    watch->setBrightness(prmBrightness);                                        //On definie la luminosité
}
//##############//

//SCREEN FUNCTION//
//Efface l'ecran
void Watch::ClearScreen(uint prmColor){
    //Variables
    //Programme
    screen->fillScreen(prmColor);                                                                           //On replis l'ecran d'une couleur
}
//Affichage de l'image
void Watch::DisplayImage(const uint16_t prmPtrImg[],uint prmWidth,uint prmHeight,uint prmX,uint prmY){
    //Variables
    //Programme
    screen->pushImage(prmX,prmY,prmWidth,prmHeight,prmPtrImg);                                          //On affiche l'image
}
//Definie la taille de la police
void Watch::SetTextFont(uint prmSize){
    //Variables
    //Programme
    screen->setTextFont(prmSize);                                                                       //Definition de la taille de texte
}
//Definie la couleur du texte et du fond
void Watch::SetTextColor(uint prmTextColor,uint prmBackColor){
    //Variables
    //Programme
    screen->setTextColor(prmTextColor,prmBackColor);                                                    //Definition de la couleur du texte et du fond
}
//Definie la position d'ecriture
void Watch::SetCursorPosition(uint prmPosX,uint prmPosY){
    //Variables
    //Programme
    screen->setCursor(prmPosX,prmPosY);                                                                 //Definition de la position d'ecriture
}
//Afficher des caracteres a l'ecran
void Watch::Print(String prmMsg){
    //Variables
    //Programme
    screen->print(prmMsg);                                                                              //Affichage sur l'ecran
}
//Si il y a une pression
bool Watch::CheckTouch(){
    //Variables
    //PRogramme
    return watch->getTouch(touchPosX,touchPosY);                                                        //Check si il y a une pression
}
//Recuperation de la position de la pression X
uint Watch::GetTouchX(){
    //Variables
    //Programme
    return touchPosX;                                                                                   //Retourne position X de la pression
}
//Recuperation de la position de la pression Y
uint Watch::GetTouchY(){
    //Variables
    //Programme
    return touchPosY;                                                                                   //Retourne position Y de la pression
}
//Change la rotation de l'ecran (voir SCREEN_ROTATION_####)
void Watch::SetScreenRotation(uint prmRotation){
    //Variables
    //Programme
    screen->setRotation(prmRotation);                                                               //Definition de la rotation
}
//###############//

//MOTOR FUNCTION//
//Lance une vibration de [ms] milliseconde
void Watch::Vibrate(uint ms){
    //Variables
    uint timeout;                                                                                   //Temps limite de la vibration
    //Programme
    timeout = millis() + ms;                                                                        //On prend le nombre de ms actuel et on ajoute le nombre de ms a faire
    while(timeout>millis()){                                                                        //Tant que l'on a pas depassé la limite de temps
        motor->onec();                                                                                  //On allume le vibreur
    }
}
//##############//

//POWER FUNCTION//
//Si la montre est en charge
bool Watch::IsCharging(){
    //Variables
    //Programme
    return power->isChargeing();                                                                    //Retourne si la montre est en chagre
}
//Recuperation du niveau de batterie
int Watch::GetBatteryPercentage(){
    //Variables
    //Programme
    return power->getBattPercentage();                                                              //Retourne le pourcentage de batterie
}
//Lecture des intéruptions
void Watch::ReadIRQ(){
    //Variables
    //Programme
    power->readIRQ();                                                                               //On regarde le type d'interuption

    irqShortPress = power->isPEKShortPressIRQ();                                                    //Si le bouton a été presser

    power->clearIRQ();                                                                              //Vide la liste des Interruptions
}
//##############//

//RTC FUNCTION//
//Recuperation de la date format (2019-08-12/15:00:56)
String Watch::GetRTC(){
    //Variables
    String timeData(rtc->formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S));                          //Recuperation des données du RTC au format (2019-08-12/15:00:56)
    //Programme
    return timeData;                                                                                //On retourne la donnée
}
//Definir l'heure du RTC
void Watch::SetRTC(uint prmYear,uint prmMonth,uint prmDay,uint prmHour,uint prmMinute,uint prmSecond){
    //Variables
    //Programme
    rtc->setDateTime(prmYear,prmMonth,prmDay,prmHour,prmMinute,prmSecond);                          //Definition de l'heure de l'horloge interne
}
//Allumé ou etteindre l'alarme
void Watch::EnableAlarm(bool prmEnable){
    //Variables
    //Programme
    if(prmEnable){                                                                                  //Si on doit allumé l'alarme
        rtc->enableAlarm();                                                                             //On allume l'alarm
    }else{                                                                                          //Sinon
        rtc->disableAlarm();                                                                            //On etteind l'alarme
    }
}
//Definir l'alarm (-1 = pas pris en compte)
void Watch::SetAlarm(int8_t prmMinute,int8_t prmHour,int8_t prmDay,int8_t prmWeekDay){
    //Variables
    //Programme
    if(prmMinute!=-1 && prmHour==-1 && prmDay==-1 && prmWeekDay==-1){                               //Si il n'y a que les minutes
        rtc->setAlarmByMinutes(prmMinute);
    }else if(prmMinute==-1 && prmHour!=-1 && prmDay==-1 && prmWeekDay==-1){                         //Sinon il n'y a que les heures
        rtc->setAlarmByHours(prmHour);
    }else if(prmMinute==-1 && prmHour==-1 && prmDay!=-1 && prmWeekDay==-1){                         //Sinon il n'y a que les jours
        rtc->setAlarmByDays(prmDay);
    }else if(prmMinute==-1 && prmHour==-1 && prmDay==-1 && prmWeekDay!=-1){                         //Sinon il n'y a que les WeekDay
        rtc->setAlarmByWeekDay(prmWeekDay);
    }else{                                                                                          //Sinon
        rtc->setAlarm(prmHour,prmMinute,prmDay,prmWeekDay);                                             //Definition de l'alarm   
    }
}
//Reset les alarms
void Watch::ResetAlarm(){
    //Variables
    //Programme
    rtc->resetAlarm();                                                                                  //Reset des alarms
}

//############//

//SENSOR//
//Recuperation du gyroscope X
int Watch::GetGyroX(){
    //Variables
    Accel accelerator;                                                                              //Données du gyroscope
    //Programme
    if(sensor->getAccel(accelerator)){                                                              //Si on a reussi a récupérer les données du gyroscope
        return accelerator.x;                                                                           //Accelerometre X
    }else{
        Serial.println("getAccel X FAIL");
        return 0;
    }
}
//Recuperation du gyroscope Y
int Watch::GetGyroY(){
    //Variables
    Accel accelerator;                                                                              //Données du gyroscope
    //Programme
    if(sensor->getAccel(accelerator)){                                                              //Si on a reussi a récupérer les données du gyroscope
        return accelerator.y;                                                                           //Accelerometre Y
    }else{
        Serial.println("getAccel Y FAIL");
        return 0;
    }
}
//Recuperation du gyroscope Z
int Watch::GetGyroZ(){
    //Variables
    Accel accelerator;                                                                              //Données du gyroscope
    //Programme
    if(sensor->getAccel(accelerator)){                                                              //Si on a reussi a récupérer les données du gyroscope
        return accelerator.z;                                                                           //Accelerometre z
    }else{
        Serial.println("getAccel Z FAIL");
        return 0;
    }
}
//Recuperation de la rotation actuel (voir SENSOR_DIRECTION_###)
int Watch::GetRotation(){
    //Variables
    //Programme
    return sensor->direction();                                                                                     //On retourne la rotation actuel
}
//Reinitialisation du nombre de pas
void Watch::ResetStepCount(){
    //Variables
    //PRogramme
    sensor->resetStepCounter();                                                                 //Reinitialisation du nombre de pas
}
//Recuperation du nombre de pas
uint Watch::GetStepCount(){
    //Variables
    //Programme
    while(!sensor->readInterrupt()){};                                                              //Tant qu'on n'a pas encore recus l'interruption
    return sensor->getCounter();                                                                    //On retourne le nombre de pas
}

//######//