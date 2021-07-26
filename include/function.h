#include <Arduino.h>

#include "config.h"                                                                             //Configuration des librairies
#include "wallpaper.h"                                                                          //Liste des images de fond

//Lance une vibration de {ms} milliseconde
void Vibrate(uint ms,TTGOClass *prmTTGO){
    //Variables
    uint timeout;                                                                                   //heure limite de la vibration    
    //Programme
    timeout = millis() + ms;                                                                        //On prend le nombre de ms actuel et on ajoute le nombre de ms a faire
    while(timeout>millis()){                                                                        //Tant que l'on a pas depassé la limite de temps
        prmTTGO->motor->onec();                                                                         //On allume le vibreur
    }
}

//Affiche une image
void DisplayImage(uint prmImageID,uint prmX,uint prmY,TTGOClass *prmTTGO){
    //Variables
    //Programme
    switch (prmImageID){                                                                            //Pour chaque images disponible
        case 1:                                                                                         //Image 1 (SCP)
            prmTTGO->tft->pushImage(prmX,prmY,Wallpaper_1_width,Wallpaper_1_height,Wallpaper_1);            //On affiche l'image
            break;
    }
}

//Gestion du retro eclairage (prmBrightness [0,255])
void SetBackLight(bool prmState,uint prmBrightness,TTGOClass *prmTTGO){
    //Variables
    //Programme
    if(prmState)prmTTGO->openBL();                                                                      //Si on allume
    if(!prmState)prmTTGO->closeBL();                                                                    //Si on eteint
    prmTTGO->setBrightness(prmBrightness);                                                              //On definie la luminosité
}

//Definir l'heure du RTC
void SetRTC(uint prmYear,uint prmMonth,uint prmDay,uint prmHour,uint prmMinute,uint prmSecond,TTGOClass *prmTTGO){
    //Variables
    //Programme
    prmTTGO->rtc->setDateTime(prmYear,prmMonth,prmDay,prmHour,prmMinute,prmSecond);
}
//Recuperation de la date format (2019-08-12/15:00:56)
String GetRTC(TTGOClass *prmTTGO){
    //Variables
    String timeData(prmTTGO->rtc->formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S));                        //On recupere les données du RTC au format (2019-08-12/15:00:56)
    //Programme
    return timeData;
}