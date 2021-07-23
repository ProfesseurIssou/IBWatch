#include <Arduino.h>

#include "config.h"                                                                             //Configuration des librairies

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