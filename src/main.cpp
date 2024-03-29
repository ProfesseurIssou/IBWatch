// https://projetsdiy.fr/ttgo-t-watch-debuter-librairie-esp32-tft_espi/
#include <Arduino.h>
#include <Watch.h>

#include "wallpaper.h"                                                                          //Liste des images de fond
#include "color.h"                                                                              //Couleur en HEX (COLOR_###)

//CONFIG//
#define SERIAL_SPEED 115200                                                                     //Vitesse de communication
//######//

//CLASS//
Watch *myWatch;
//#####//

//VARIABLES//
bool wallpaperDisplayed;                                                                    //Si le fond d'ecran est déjà affiché
uint ss,mn,hh,dd,mm,yy;                                                                     //Variables pour les dates et les heures
byte minuteCache = 99;                                                                      //Variable pour evite le refresh constant de l'heure
uint rotation;                                                                              //Rotation de l'ecran
uint rotationCache = SENSOR_DIRECTION_BOTTOM;                                               //Ancienne rotation de l'ecran
//#########//

void setup(){
    Serial.begin(SERIAL_SPEED);                                                                     //On definie la vitesse de communication
    Serial.println("START");
    
    myWatch = new Watch(false);                                                                     //Creation du gestionnaire de la montre (pas d'accelerometre)

    myWatch->SetBackLight(true,255);                                                                //On demarre le retroeclairage
    myWatch->Vibrate(10);                                                                           //On lance la vibration de demarrage
    wallpaperDisplayed = false;                                                                     //Pas de fond encore afficher

    // myWatch->EnableAlarm(false);                                                                    //On arrete l'alarme
    // myWatch->SetAlarm(10,-1,-1,-1);                                                                  //On definie une alarme
    // myWatch->EnableAlarm(true);                                                                     //On demarre l'alarme

    delay(5000);                                                                                    //5000ms
}

//Sync du rtc
void WifiSyncRTC(){
    //Variables
    //Programme
    myWatch->ConnectToWifi("SSID","PASSWORD");                                  //Connection au wifi
    while(myWatch->WifiConnected()){                                                                //Tant que le wifi n'est pas connecté
        delay(500);                                                                                     //500ms
    }
    myWatch->SetCursorPosition(150,225);                                                            //On ecrit en bas de l'ecran
    myWatch->SetTextColor(TFT_GREEN,TFT_BLACK);                                                     //Text vert fond noir
    if(!myWatch->SyncTimeWifi()){                                                                   //Si on n'arrive pas a synchronisé le wifi
        myWatch->SetCursorPosition(150,225);                                                            //On ecrit en bas de l'ecran
        myWatch->Print("Sync Failed ");
        delay(1000);                                                                                    //1000ms
        myWatch->SetCursorPosition(150,225);                                                            //On ecrit en bas de l'ecran
        myWatch->Print("Sync        ");
    }
    myWatch->SetCursorPosition(150,225);                                                            //On ecrit en bas de l'ecran
    myWatch->Print("Synchronized");
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
    /*ROTATION*/
    // rotation = myWatch->GetRotation();                                                              //On recupere la rotation actuel du sensor
    // if(rotationCache != rotation){                                                                  //Si il y a une nouvelle rotation
    //     switch(rotation){                                                                               //Pour chaque valeur de rotation
    //         case SENSOR_DIRECTION_BOTTOM:                                                                   //Si on est vers le bas
    //             myWatch->ClearScreen(COLOR_BLACK);                                                              //On remplis l'ecran en noir
    //             wallpaperDisplayed = false;                                                                     //Il faut réafficher le fond d'ecran
    //             minuteCache = 99;                                                                               //Il faut réafficher l'heure
    //             rotationCache = rotation;                                                                       //On met en cache l'ancienne rotation
    //             myWatch->SetScreenRotation(SCREEN_ROTATION_DOWN);                                               //On met la rotation vers le bas
    //             break;
    //         case SENSOR_DIRECTION_TOP:                                                                      //Si on est vers le haut
    //             myWatch->ClearScreen(COLOR_BLACK);                                                              //On remplis l'ecran en noir
    //             wallpaperDisplayed = false;                                                                     //Il faut réafficher le fond d'ecran
    //             minuteCache = 99;                                                                               //Il faut réafficher l'heure
    //             rotationCache = rotation;                                                                       //On met en cache l'ancienne rotation
    //             myWatch->SetScreenRotation(SCREEN_ROTATION_UP);                                                 //On met la rotation vers le haut
    //             break;
    //     }
    // }
    /*########*/

    /*WALLPAPER*/
    if(!wallpaperDisplayed){                                                                        //Si le fond d'ecran n'est pas encore affiché
        myWatch->DisplayImage(Wallpaper_1,Wallpaper_1_width,Wallpaper_1_height,0,0);                    //Affichage de l'image de fond
        wallpaperDisplayed = true;                                                                      //Le fond d'ecran est affiché
    }
    notificationBar();                                                                              //Affichage de la barre des notification
    /*#########*/

    /*STEP COUNTER*/
    // if(myWatch->StepDetected()){                                                                    //Si il y a des pas
    //     myWatch->SetStepDetected(false);                                                                //On remet le trigger a False
    //     myWatch->SetCursorPosition(50,50);                                                              //On met le curseur d'ecriture en 50*50
    //     myWatch->Print(String(myWatch->GetStepCount()));                                                //On affiche le nombre de pas
    // }
    /*############*/

    /*TOUCH SCREEN*/
    if(myWatch->CheckTouch()){                                                                      //Si il y a une pression sur l'ecran
        Serial.print("Change Mode ");
        Serial.print(myWatch->GetTouchX());
        Serial.print(" ");
        Serial.println(myWatch->GetTouchY());
        WifiSyncRTC();                                                                                  //Sync RTC avec le wifi
        delay(100);
    }
    /*############*/

    /*ALARM*/
    if(myWatch->AlarmRing()){                                                                       //Si l'alarm sonne
        myWatch->SetAlarmRing(false);                                                                   //On desactive l'alarme
        myWatch->Vibrate(100);                                                                          //On lance une vibration
        myWatch->ResetAlarm();                                                                          //On reset l'alarm
        myWatch->EnableAlarm(false);                                                                    //On arrete l'alarme
    }
    /*#####*/

    /*BOUTON PRESSER*/
    if(myWatch->ButtonPressed()){                                                                   //Si le bouton est presser
        myWatch->SetButtonPressed(false);                                                           //On remet la variable trigger à False
        myWatch->ReadIRQ();                                                                         //On lance la recuperation des Interruptions       
        if(myWatch->GetIRQShortPress()){                                                            //Si l'interuption est une pression rapide
            myWatch->DeepSleep(true,false);                                                             //On lance le deepsleep avec un allumage bouton            
        }
        delay(1000);
    }
    /*##############*/

    /*WIFI*/    
    myWatch->SetCursorPosition(0,225);                                                              //On ecrit en bas de l'ecran
    myWatch->SetTextColor(TFT_GREEN,TFT_BLACK);                                                     //Text vert fond noir
    if(myWatch->WifiConnected()){                                                                   //Si le wifi est connecté
        myWatch->Print("CONNECTED    ");                                                                //Print
    }else{                                                                                          //Sinon
        myWatch->Print("NOT CONNECTED");                                                                //Print
    }
    /*####*/


    delay(100);
}
