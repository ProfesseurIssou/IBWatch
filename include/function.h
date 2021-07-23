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

//Gestion du retro eclairage
void SetBackLight(bool prmState,TTGOClass *prmTTGO){
    if(prmState)prmTTGO->openBL();                                                                      //Si on allume
    if(!prmState)prmTTGO->closeBL();                                                                    //Si on eteint
}