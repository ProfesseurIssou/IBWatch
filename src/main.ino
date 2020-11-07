#include "config.h"

TTGOClass *ttgo;
TFT_eSPI *tft;
AXP20X_Class *power;

/*
code    color
0x0000  Black
0xFFFF  White
0xBDF7  Light Gray
0x7BEF  Dark Gray
0xF800  Red
0xFFE0  Yellow
0xFBE0  Orange
0x79E0  Brown
0x7E0   Green
0x7FF   Cyan
0x1F    Blue
0xF81F  Pink
*/

bool irq = false;               //Si on doit couper l'ecrant, le bouton (irq = interupt)
unsigned int screenMode = 0;    //Quel mode somme nous (0 = MainMenu)
bool screenDisplay = true;      //Si on allume l'ecran
unsigned int vibratorLimit = 0; //La limite en milliseconde que le moteur doit fonctionner (si l'horloge en milli de l'esp est supérieur alors il stop
#define boutonPin AXP202_INT    //Simplification

/*Pour l'heure*/
static uint8_t conv2d(const char *p){
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}
unsigned int targetss,targetmm,targethh;//le temp a ajouter au compteur
byte minuteCache = 99;         // Pour l'affichage de l'heure (evité l'effet stroboscope)
uint8_t basehh = conv2d(__TIME__), basemm = conv2d(__TIME__ + 3), basess = conv2d(__TIME__ + 6); // Get H, M, S from compile time
uint8_t hh,mm,ss;             //Le temp à ajouter
//Convert all data into second
unsigned int baseSec = (basehh * 3600) + (basemm * 60) + basess;

void setup() {
  Serial.begin(115200);
  ttgo = TTGOClass::getWatch();                   //Récuperation des instance de la montre
  ttgo->begin();                                  //On initialise le materiel
  ttgo->openBL();                                 //On démarre la lumiere de l'ecran
  ttgo->motor_begin();                            //On démarre le moteur de vibration pin IO4
  tft = ttgo->tft;                              //Simplification
  power = ttgo->power;                          //Simplification
  
  //On met en route le moniteur de puissance
  power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);

  pinMode(boutonPin, INPUT_PULLUP);                     //On definie le bouton comme entrée
  attachInterrupt(boutonPin, [] {irq = true;}, FALLING);//Si le bouton est presser alors on passe la variable irq a True
  
  power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);    //On met en place le systeme d'interuption grace au bouton
  power->clearIRQ();                                    //On vide l'interuption
 
  tft->fillScreen(TFT_BLACK);                           //On met l'ecrant en noir
  Serial.println("START");
}

//Calculer l'heure
void timeCalc(){
  //calcul du temps d'execution
  targetss = (millis() / 1000) + baseSec;               //nb second from start
  targetmm = ((millis() / 1000) + baseSec) /60;         //take minute from start
  targethh = ((millis() / 1000) + baseSec) /3600;       //take hour from start
  ss = targetss % 60;
  mm = targetmm % 60;
  hh = targethh % 24;
}

//Utilisation du moteur pour les vibration
void setVibrator(unsigned int timer){
  vibratorLimit = millis() + timer;
  Serial.print("VibratorLimit : ");
  Serial.println(vibratorLimit);
}
void vibrator(){
  if (vibratorLimit > millis()){            //Si on doit allumer le vibreur
    Serial.println("Vibrator");
    ttgo->motor->onec();                    //On allume le vibreur
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

/*Barre des notification*/
void notificationBar(){
  //Rectangle en haut de l'ecran
  
  /*Affichage de l'heure*/
  if (minuteCache != mm) { //Si les minute on changer
    Serial.println("reload hour");
    displayClock();
  }

  /*Affichage de la batterie*/
  displayBatterie();
}
void displayClock(){
  tft->setTextFont(2);                    //On met la taille de l'ecriture a 2
  tft->setTextColor(0xFFFF, TFT_BLACK);   //On choisie la couleur du texte a orange et le fond en noir
  tft->setCursor(0, 0);                   //On met le curseur en haut à gauche
  minuteCache = mm;                       //On remet le cache à jour
  
  /*Eviter les caratère parasite (réafficher un caratère vide)*/
  if (hh < 10){                                       //Si l'heure est sur 1 chiffre
    tft->print(" ");                                  //On affiche un caratère vide
  }
  tft->print(hh);                                     //On affiche l'heure
  tft->print(":");                                    //On affiche le séparateur heure/minute
  if (mm < 10){                                       //Si les minutes est sur 1 chiffre
    tft->print("0");                                  //On affiche un 0
  }
  tft->print(mm);                                     //On affiche les minute
}
void displayBatterie(){
  tft->setTextFont(2);                    //On met la taille de l'ecriture a 2
  tft->setCursor(200, 0);                 //On met le curseur en haut à gauche

  if (power->isChargeing()) {             //Si le chargeur est brancher
    tft->setTextColor(0x7E0, TFT_BLACK); //On choisie la couleur du texte a vert et le fond en noir
  } else {
    tft->setTextColor(0xFFFF, TFT_BLACK); //On choisie la couleur du texte a blanc et le fond en noir
  }
  tft->print(power->getBattPercentage()); //On affiche la batterie
  tft->println("%");                      //On affiche le logo%
}

//Affichage du menu principale
void mainMenu(){
  /*Affichage de la bar de notification*/
  notificationBar();
}

void loop(){
  screenDisplayer();                        //On verifie si l'ecran doit etre alumé ou non
  timeCalc();                               //Calcule de l'heure
  vibrator();                               //On verifie si on doit utilisé le vibreur

  if (screenDisplay == true){               //Si l'ecran est allumé
    if (screenMode == 0){
      mainMenu();                           //On affiche le menu principale
    }
    
    /*Si on appuis sur l'ecran*/
    int16_t x, y;
    if (ttgo->getTouch(x, y)) {
      Serial.println("Change Mode");
      delay(100);
    }
  }


  /*bouton pressé*/
  if (irq) {
    irq = false;                          //on remet la variable trigger à False
    power->readIRQ();                     //on regarde le type d'interuption
    if (power->isPEKShortPressIRQ()) {    //si l'interuption est une pression rapide
      if (screenDisplay == true){         //Si l'ecran est allumer alors on l'eteint
        Serial.println("Turn off screen");
        screenDisplay = false;            //On eteint l'ecran
        setVibrator(200);                 //On vibre pendant 0.2 seconde
      }else{                              //Sinon l'ecran est eteind alors on l'allume
        Serial.println("Turn on screen");
        screenDisplay = true;             //On allume l'ecran
        setVibrator(200);                 //On vibre pendant 0.2 seconde
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
    power->clearIRQ();
  }
  
  delay(100);
}
