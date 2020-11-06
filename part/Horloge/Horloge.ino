#include "config.h"

TTGOClass *ttgo;
TFT_eSPI *tft;

/*Pour l'heure*/
unsigned int targetss,targetmm,targethh;//le temp a ajouter au compteur
byte minuteCache = 99;         // Pour l'affichage de l'heure (evité l'effet stroboscope)
static uint8_t conv2d(const char *p){
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}
uint8_t basehh = conv2d(__TIME__), basemm = conv2d(__TIME__ + 3), basess = conv2d(__TIME__ + 6); // Get H, M, S from compile time
uint8_t hh,mm,ss;             //Le temp à ajouter

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
  
  //Preparation de l'ecran
  //On met l'ecrant en noir
  tft->fillScreen(TFT_BLACK); 
  //...
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
  //...
  //Calcule de l'heure
  timeCalc();
  clockScreen();  
  //...
}
