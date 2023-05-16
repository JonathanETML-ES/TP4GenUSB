// Mc32Gest_SerComm.C
// fonction d'émission et de réception des message
// transmis en USB CDC
// Canevas TP4 SLO2 2015-2015

#include "app.h"
#include "Mc32gest_SerComm.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


// Fonction de reception  d'un  message
// Met à jour les paramètres du generateur a partir du message recu
// Format du message
//  !S=CF=2000A=19000O=+1100W=1#
//  !S=CF=2000A=19000O=-1100W=1#
//  !S=CF=2000A=19000O=+1100W=0#
//  !S=CF=2000A=19000O=-1100W=0#
void GetMessage(uint8_t *USBReadBuffer, S_ParamGen *pParam, bool *SaveTodo)
{
    // Variable temporaire d'enregistrement de caractères
    char Forme[2];
    char Frequence[5];
    char Amplitude[6];
    char Signe_Offset[2];
    char Offset[5];
    char SaveW[2];
    
    // Enregistrement des caractères issu du Buffer
    sscanf((char*)USBReadBuffer, "!S=%1sF=%4sA=%5sO=%1s%4sW=%1s#", 
            Forme, Frequence, Amplitude, Signe_Offset, Offset, SaveW);
    
    // Selection de la forme du signal et mis dans notre structure pParam
    switch(Forme[0])
    {
        case 'S':
            pParam->Forme = SignalSinus;
            break;
        case 'T':
            pParam->Forme = SignalTriangle;
            break;
        case 'D':
            pParam->Forme = SignalDentDeScie;
            break;
        case 'C':
            pParam->Forme = SignalCarre;
            break;
    }
    // Conversion des caractere ressortie avec le sscanf et mis les valeurs dans notre structure pParam
    pParam->Amplitude = (atoi(Amplitude));
    pParam->Frequence = (atoi(Frequence));
    
    if(Signe_Offset[0] == '+') pParam->Offset = atoi(Offset);
    else pParam->Offset = 0 - (atoi(Offset));
    
    //Ressortie la valeur de la variable de la sauvegarde
    *SaveTodo = (atoi(SaveW));
} // GetMessage


// Fonction d'envoi d'un  message
// Rempli le tampon d'émission pour USB en fonction des paramètres du générateur
// Format du message
// !S=CF=2000A=19000O=+1100WP=1#
// !S=CF=2000A=19000O=-1100WP=1#
// !S=CF=2000A=19000O=+1100WP=0#
// !S=CF=2000A=19000O=-1100WP=0#
void SendMessage(uint8_t *USBSendBuffer, S_ParamGen *pParam, bool Saved )
{
    // Variable temporaire reprenant les valeurs de notre structure pParam
    E_FormesSignal Forme = pParam->Forme;
    int16_t Frequence = pParam->Frequence;
    int16_t Ampere = pParam->Amplitude;
    int16_t Offset = pParam->Offset;
    
    int WP = Saved;
    char FormeSignal;
    
    // Selection de la forme du signal
    switch(Forme)
    {
        case SignalSinus:
            FormeSignal = 'S';
            break;
        case SignalTriangle:
            FormeSignal = 'T';
            break;
        case SignalDentDeScie:
            FormeSignal = 'D';
            break;
        case SignalCarre:
            FormeSignal = 'C';
            break;
    }
    
    // Envoie sur le buffeur lorsque nous avons un offset positif
    if(pParam->Offset >= 0) sprintf((char*)USBSendBuffer, "!S=%cF=%4iA=%5iO=+%4iWP=%1i#", 
            FormeSignal, Frequence, Ampere, Offset, WP);
    // Envoie sur le buffeur lorsque nous avons un offset négatif
    else sprintf((char*)USBSendBuffer, "!S=%cF=%4iA=%5iO=%4iWP=%1i#", 
            FormeSignal, Frequence, Ampere, Offset, WP);
} // SendMessage
