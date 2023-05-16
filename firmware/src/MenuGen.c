// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  10/02/2015 pour SLO2 2014-2015
// Fichier MenuGen.c
// Gestion du menu  du générateur
// Traitement cyclique à 10 ms



#include <stdint.h>                   
#include <stdbool.h>
#include "MenuGen.h"
#include "GesPec12.h"
#include "Generateur.h"

#include "bsp.h"
#include "Mc32NVMUtil.h"
#include "GenericTypeDefs.h"
#include "Mc32DriverLcd.h"
#include "Mc32Delays.h"

const char MenuFormes[4][21] = { "Sinus      ", "Triangle   ", "DentDeScie ", "Carre      " };

uint8_t flag_menu;
uint8_t flag_ampl;
uint8_t flag_freq;
uint8_t flag_offset;

void MENU_DemandeSave(void)
{
    
}

// Initialisation du menu et des paramètres
void MENU_Initialize(S_ParamGen *pParam)
{
    //Lecture flash
    
    //restauration des paramètres si lecture ok, sinon parm par défaut.
    
    //pour le moment, param. par défaut
    pParam->Forme = SignalSinus;
    pParam->Frequence = 1000;
    pParam->Amplitude = 5000;
    pParam->Offset = 0;
    pParam->Magic = MAGIC;
    
    GENSIG_UpdateSignal(pParam);
}


// Execution du menu, appel cyclique depuis l'application
void MENU_Execute(S_ParamGen *pParam, bool local)
{
    if(local == true)
    {
        static int32_t n = 1;
        static int32_t x = 0;
        static int32_t y = 0;
        static int32_t Etat = 1;
        static int Flag_refresh = 0;

        // Lecture du PEC12 pour naviguer dans le menu
        if(flag_menu == 0)
        {     
           if (Pec12IsPlus()) 
            {
                Pec12ClearPlus();
                lcd_bl_on();
                n++;     
            }

            if (Pec12IsMinus())
            {
                Pec12ClearMinus();
                lcd_bl_on();
                n--;
            }
          Etat = n;  
        }

        if(flag_menu == 1)
        {
            if (Pec12IsPlus()) 
            {
                Pec12ClearPlus(); 
                y++;

                if(flag_freq == 1)
                {
                    pParam->Frequence += 20;

                    if(pParam->Frequence > 2000)
                        pParam->Frequence = 20;
                }

                if(flag_ampl == 1)
                {
                    pParam->Amplitude += 100;

                    if(pParam->Amplitude > 10000)
                    {
                        pParam->Amplitude = 0;
                    }

                }

                if(flag_offset == 1)
                {
                    pParam->Offset += 100;

                    if(pParam->Offset > 5000)
                        pParam->Offset = 5000;
                }
            }

            if (Pec12IsMinus())
            {
                Pec12ClearMinus();
                y--;

                if(flag_freq == 1)
                {
                    pParam->Frequence -= 20;

                    if(pParam->Frequence < 20)
                        pParam->Frequence = 2000;
                }

                if(flag_ampl == 1)
                {
                    if(pParam->Amplitude == 0)
                        pParam->Amplitude = 10100;
                    pParam->Amplitude -= 100;
                }

                if(flag_offset == 1)
                {
                    pParam->Offset -= 100;

                    if(pParam->Offset < -5000)
                        pParam->Offset = -5000;
                }
            }
          Etat = n;  
        }

        // Sécuritée pour l'étoile
        if(n >= 4)
        {
            n = 4;
        }

        if(n <= 0)
        {
            n = 1;
        }



        // Contrôle du pointeur (étoile))
        switch(Etat)
        {
            case 1:

                lcd_gotoxy(1,1);
                printf_lcd("*");
                lcd_gotoxy(1,2);
                printf_lcd(" ");
                lcd_gotoxy(1,3);
                printf_lcd(" ");
                lcd_gotoxy(1,4);
                printf_lcd(" ");

                if(Pec12IsOK())
                {
                    Pec12ClearOK();
                    x = 1; 
                    flag_menu = 1;
                    Flag_refresh = 1;
                }

                break;

            case 2 :
                lcd_gotoxy(1,2);
                printf_lcd("*");
                lcd_gotoxy(1,1);
                printf_lcd(" ");
                lcd_gotoxy(1,3);
                printf_lcd(" ");
                lcd_gotoxy(1,4);
                printf_lcd(" ");

                if(Pec12IsOK())
                {
                    Pec12ClearOK();
                    x = 2;
                    flag_menu = 1;
                    Flag_refresh = 1;
                }
                break;

            case 3:
                lcd_gotoxy(1,3);
                printf_lcd("*");
                lcd_gotoxy(1,1);
                printf_lcd(" ");
                lcd_gotoxy(1,2);
                printf_lcd(" ");
                lcd_gotoxy(1,4);
                printf_lcd(" ");

                if(Pec12IsOK())
                {
                    Pec12ClearOK();
                    x = 3;
                    flag_menu = 1;
                    Flag_refresh = 1;
                }

                break;

            case 4 : 
                lcd_gotoxy(1,4);
                printf_lcd("*");
                lcd_gotoxy(1,1);
                printf_lcd(" ");
                lcd_gotoxy(1,2);
                printf_lcd(" ");
                lcd_gotoxy(1,3);
                printf_lcd(" ");

                if(Pec12IsOK())
                {
                    Pec12ClearOK();
                    x = 4;
                    flag_menu = 1;
                    Flag_refresh = 1;
                }
                break; 
                
            default :
                break;
        }

        if(Flag_refresh)
        {
        // Mise à jour des valeurs de tension, fréquence, amplitude
        switch(x)
        {
             case 1:

                 lcd_gotoxy(1,1);
                 printf_lcd("?");

                 lcd_gotoxy(2,1);
                 printf_lcd("Forme = %s", MenuFormes[y]);

                 // Limite du choix des signaux
                 if(y > 3)
                 {
                     y = 0;
                 }

                 if(y < 0)
                 {
                     y = 3;
                 }

                 // Retour dans le premier switch case (choix de l'étoile))
                 if(Pec12IsOK())
                 {
                     Pec12ClearOK();
                     n = 1;
                     flag_menu = 0;

                     switch(y)
                     {
                         case 0:
                             pParam->Forme = SignalSinus;
                             break;

                         case 1:
                             pParam->Forme = SignalTriangle;
                             break;

                         case 2:
                             pParam->Forme = SignalDentDeScie;
                             break;

                         case 3:
                             pParam->Forme = SignalCarre;
                             break;
                     }
                     GENSIG_UpdateSignal(pParam);
                     Flag_refresh = 0;
                 }
                 break;

             case 2 :
                 flag_freq = 1; 

                 lcd_gotoxy(1,2);
                 printf_lcd("?");

                 lcd_gotoxy(2,2);
                 printf_lcd("Freq [Hz] = %4d", pParam->Frequence);

                 if(Pec12IsOK())
                 {
                     Pec12ClearOK();
                     n = 2;
                     flag_menu = 0;
                     flag_freq = 0; 

                     GENSIG_UpdatePeriode(pParam);
                     Flag_refresh = 0;
                 }
                 break;

             case 3:
                 flag_ampl = 1;

                 lcd_gotoxy(1,3);
                 printf_lcd("?");


                 lcd_gotoxy(2,3);
                 printf_lcd("Ampl [mV] = %5d", pParam->Amplitude);

                 // Retour dans le premier switch case (choix de l'étoile))
                 if(Pec12IsOK())
                 {
                     Pec12ClearOK();
                     n = 3;
                     flag_menu = 0;
                     flag_ampl = 0;

                     GENSIG_UpdateSignal(pParam);
                     Flag_refresh = 0;
                 }
                 break;

             case 4 : 
                 flag_offset = 1;

                 lcd_gotoxy(1,4);
                 printf_lcd("?");


                 lcd_gotoxy(2,4);
                 printf_lcd("Offset [mV] = %5d", pParam->Offset);    

                 // Retour dans le premier switch case (choix de l'étoile))
                 if(Pec12IsOK())
                 {
                     Pec12ClearOK();
                     n = 4;
                     flag_menu = 0;
                     flag_ampl = 0;


                     GENSIG_UpdateSignal(pParam);
                     Flag_refresh = 0;
                 }
                 break;

             default :
                 break;

        }
      }   
    }
    else GENSIG_UpdateSignal(pParam);
}







