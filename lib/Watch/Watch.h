#pragma once
#include <Arduino.h>

#include "config.h"                                                                             //Configuration des librairies TTGO

#define SIZE_SCREEN_X 240
#define SIZE_SCREEN_Y 240

//PIN//
#define RTC_PIN RTC_INT_PIN
#define BUTTON_PIN AXP202_INT                                                                   //Pin du bouton lateral
#define BUZZER_PIN 25                                                                           //Pin du buzzer
#define BUZZER_CHANNEL 0
//###//


class Watch{
    public:
        //INIT/
        Watch();                                                                                //Contructeur
        ~Watch();                                                                               //Destructeur
        void DeepSleep(bool prmWakeButton,bool prmWakeTouch);                                   //Lance le deep sleep de la montre
        //####//

        //GETTER SETTER//
        bool ButtonPressed();                                                                   //Si le bouton est pressé
        void SetButtonPressed(bool prmState);                                                   //Definition de l'etat du bouton
        bool AlarmRing();                                                                       //Si l'alarme sonne
        void SetAlarmRing(bool prmState);                                                       //Definition de l'etat de l'alarme
        bool GetIRQShortPress();                                                                //Si le bouton a été pressé
        //#############//

        //WATCH FUNCTION//
        void SetBackLight(bool prmState,uint prmBrightness=255);                                //Gestion du retro eclairage (prmBrightness [0,255])
        //##############//

        //SCREEN FUNCTION//
        void DisplayImage(const uint16_t prmPtrImg[],uint prmWidth,uint prmHeight,uint prmX,uint prmY);//Affichage de l'image
        void SetTextFont(uint prmSize);                                                         //Definie la taille de la police
        void SetTextColor(uint prmTextColor,uint prmBackColor);                                 //Definie la couleur du texte et du fond
        void SetCursorPosition(uint prmPosX,uint prmPosY);                                      //Definie la position d'ecriture
        void Print(String prmMsg);                                                              //Afficher des caracteres a l'ecran
        bool CheckTouch();                                                                      //Si il y a une pression
        uint GetTouchX();                                                                       //Recuperation de la position de la pression X
        uint GetTouchY();                                                                       //Recuperation de la position de la pression Y
        //###############//

        //MOTOR FUNCTION//
        void Vibrate(uint ms);                                                                  //Lance une vibration de [ms] milliseconde
        //##############//

        //POWER FUNCTION//
        bool IsCharging();                                                                      //Si la montre est en charge
        int GetBatteryPercentage();                                                             //Recuperation du niveau de batterie
        void ReadIRQ();                                                                         //Lecture des intéruptions
        //##############//

        //RTC FUNCTION//
        String GetRTC();                                                                        //Recuperation de la date format (2019-08-12/15:00:56)
        void SetRTC(uint prmYear,uint prmMonth,uint prmDay,uint prmHour,uint prmMinute,uint prmSecond);//Definir l'heure du RTC
        void EnableAlarm(bool prmEnable);                                                       //Allumé ou etteindre l'alarme
        void SetAlarm(int8_t prmMinute,int8_t prmHour,int8_t prmDay,int8_t prmWeekDay);         //Definir l'alarm (-1 = pas pris en charge)
        void ResetAlarm();                                                                      //Reset les alarms
        //############//

    private:
        //CLASS//
        TTGOClass *watch;                                                                       //Class de gestion de la montre
        TFT_eSPI *screen;                                                                       //Class de gestion de l'ecran
        AXP20X_Class *power;                                                                    //Class de gestion de la batterie
        Motor *motor;                                                                           //Class de gestion du moteur
        PCF8563_Class *rtc;                                                                     //Class de gestion de l'horloge interne
        //#####//

        //STATES//
        bool buttonPressed;                                                                     //Si le bouton est pressé (IRQ)
        bool alarmRing;                                                                         //Si l'alarme sonne (RTC IRQ)
        bool irqShortPress;                                                                     //Si le bouton a été pressé
        //######//

        //TOUCH//
        int16_t touchPosX;                                                                      //Position X de la pression
        int16_t touchPosY;                                                                      //Position y de la pression
        //#####//
};