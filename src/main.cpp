// https://projetsdiy.fr/ttgo-t-watch-debuter-librairie-esp32-tft_espi/
#include <Arduino.h>
#include <Watch.h>

#include "wallpaper.h"                                                                          //Liste des images de fond
#include "color.h"                                                                              //Couleur en HEX (COLOR_###)
// #include "function.h"                                                                           //Fonction de la montre

//CONFIG//
#define SERIAL_SPEED 115200                                                                     //Vitesse de communication
//######//

//CLASS//
Watch *myWatch;
//#####//

//VARIABLES//
bool wallpaperDisplayed;                                                                    //Si le fond d'ecran est déjà affiché
unsigned int ss,mn,hh,dd,mm,yy;                                                             //Variables pour les dates et les heures
byte minuteCache = 99;                                                                      //Variable pour evite le refresh constant de l'heure
//#########//

void setup(){
    Serial.begin(SERIAL_SPEED);                                                                     //On definie la vitesse de communication
    Serial.println("START");
    
    myWatch = new Watch();                                                                          //Creation du gestionnaire de la montre

    myWatch->SetBackLight(true,255);                                                                //On demarre le retroeclairage
    myWatch->Vibrate(10);                                                                           //On lance la vibration de demarrage
    wallpaperDisplayed = false;                                                                     //Pas de fond encore afficher

    // myWatch->EnableAlarm(false);                                                                    //On arrete l'alarme
    // myWatch->SetAlarm(10,-1,-1,-1);                                                                  //On definie une alarme
    // myWatch->EnableAlarm(true);                                                                     //On demarre l'alarme
}

//Calculer l'heure et la date
void timeCalc(){
    String timeData = myWatch->GetRTC();                                                            //On recupere les données du RTC format (2019-08-12/15:00:56)
    String date,times;                                                                              //Les variables des parties du timeData
    unsigned int pos = 0;                                                                           //La position des séparateurs

    //Separation des dates et des heures
    pos = timeData.indexOf("/");                                                                    //Recuperation de l'emplacement du séparateur
    date = timeData.substring(0,pos);                                                               //Recuperation de la partie des dates
    times = timeData.substring(pos+1,timeData.length());                                            //Recuperation de la partie du temps

    //Separation des dates
    pos = date.indexOf("-");                                                                        //Recuperation de l'emplacement du séparateur
    yy = date.substring(0,pos).toInt();                                                             //Recuperation de la partie de l'annee
    pos = date.indexOf("-");                                                                        //Recuperation de l'emplacement du séparateur
    date = date.substring(pos+1,date.length());                                                     //Recuperation du reste de la partie
    mm = date.substring(0,pos).toInt();                                                             //Recuperation de la partie du mois
    dd = date.substring(pos+1,date.length()).toInt();                                               //Recuperation de la partie des jours

    //Separation des heures
    pos = times.indexOf(":");                                                                       //Recuperation de l'emplacement du séparateur
    hh = times.substring(0,pos).toInt();                                                            //Recuperation de la partie des heures
    pos = times.indexOf(":");                                                                       //Recuperation de l'emplacement du séparateur
    times = times.substring(pos+1,times.length());                                                  //Recuperation du reste de la partie
    mn = times.substring(0,pos).toInt();                                                            //Recuperation de la partie des minutes
    ss = times.substring(pos+1,times.length()).toInt();                                             //Recuperation de la partie des secondes
}

/*Barre des notification*/
void displayClock(){
    timeCalc();                                                                                     //Calcule de l'heure
    myWatch->SetTextFont(2);                                                                        //On met la taille de l'ecriture a 2
    myWatch->SetTextColor(TFT_WHITE,TFT_BLACK);                                                     //Texte en blanc et fond en noir
    myWatch->SetCursorPosition(0,0);                                                                //On met le curseur en haut à gauche
    minuteCache = mn;                                                                               //On remet le cache à jour
  
    /*Eviter les caratère parasite (réafficher un caratère vide)*/
    if(hh<10){                                                                                      //Si l'heure est sur 1 chiffre
        myWatch->Print(" ");                                                                            //On affiche un caratère vide
    }
    myWatch->Print(String(hh));                                                                     //On affiche l'heure
    myWatch->Print(":");                                                                            //On affiche le séparateur heure/minute
    if(mn<10){                                                                                      //Si les minutes est sur 1 chiffre
        myWatch->Print("0");                                                                            //On affiche un 0
    }
    myWatch->Print(String(mn));                                                                     //On affiche les minute
}
void displayBatterie(){
    myWatch->SetTextFont(2);                                                                        //On met la taille de l'ecriture a 2
    myWatch->SetCursorPosition(200,0);                                                              //On met le curseur en haut à gauche
    if(myWatch->IsCharging()){                                                                      //Si le chargeur est brancher
        myWatch->SetTextColor(COLOR_GREEN,COLOR_BLACK);                                                 //On choisie la couleur du texte en vert et le fond en noir
    }else{                                                                                          //Sinon
        if(myWatch->GetBatteryPercentage()<20){                                                         //Si la batterie est inferieur à 20%
            myWatch->SetTextColor(COLOR_RED,COLOR_BLACK);                                                   //On choisie la couleur du texte en rouge et le fond en noir
        }else{                                                                                          //Sinon
            myWatch->SetTextColor(COLOR_WHITE,COLOR_BLACK);                                                 //On choisie la couleur du texte en blanc et le fond en noir
        }
    }
    if(myWatch->GetBatteryPercentage()<100){                                                        //Si la batterie n'a que deux chiffre
        myWatch->Print(" ");                                                                            //On affiche un espace
    }
    if(myWatch->GetBatteryPercentage()<10){                                                         //Si la batterie n'a qu'un chiffre
        myWatch->Print(" ");                                                                            //On affiche un espace
    }
    myWatch->Print(String(myWatch->GetBatteryPercentage()));                                        //On affiche la batterie
    myWatch->Print("%");                                                                            //On affiche le logo%
}
void notificationBar(){
    //Rectangle en haut de l'ecran
  
    /*Affichage de l'heure*/
    if(minuteCache!=mn){                                                                            //Si les minute on changer
        Serial.println("reload hour");
        displayClock();
    }

    /*Affichage de la batterie*/
    displayBatterie();
}

void loop(){
    if(!wallpaperDisplayed){                                                                        //Si le fond d'ecran n'est pas encore affiché
        myWatch->DisplayImage(Wallpaper_1,Wallpaper_1_width,Wallpaper_1_height,0,0);                    //Affichage de l'image de fond
        wallpaperDisplayed = true;                                                                      //Le fond d'ecran est affiché
    }

    notificationBar();                                                                              //Affichage de la barre des notification

    
    /*Si on appuis sur l'ecran*/
    if(myWatch->CheckTouch()){                                                                      //Si il y a une pression sur l'ecran
        Serial.print("Change Mode ");
        Serial.print(myWatch->GetTouchX());
        Serial.print(" ");
        Serial.println(myWatch->GetTouchY());
        delay(100);
    }

    if(myWatch->AlarmRing()){                                                                       //Si l'alarm sonne
        myWatch->SetAlarmRing(false);                                                                   //On desactive l'alarme
        myWatch->Vibrate(100);                                                                          //On lance une vibration
        myWatch->ResetAlarm();                                                                          //On reset l'alarm
        myWatch->EnableAlarm(false);                                                                    //On arrete l'alarme
    }

    /*bouton pressé*/
    if(myWatch->ButtonPressed()){                                                                   //Si le bouton est presser
        myWatch->SetButtonPressed(false);                                                           //On remet la variable trigger à False
        myWatch->ReadIRQ();                                                                         //On lance la recuperation des Interruptions       
        if(myWatch->GetIRQShortPress()){                                                            //Si l'interuption est une pression rapide
            myWatch->DeepSleep(true,false);                                                             //On lance le deepsleep avec un allumage bouton            
        }
        delay(1000);
    }
    
    delay(100);
}
