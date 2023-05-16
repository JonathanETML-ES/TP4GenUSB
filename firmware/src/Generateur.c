// Canevas manipulation GenSig avec menu
// C. HUBER  09/02/2015
// Fichier Generateur.C
// Gestion  du générateur

// Prévu pour signal de 40 echantillons

// Migration sur PIC32 30.04.2014 C. Huber
#include "Generateur.h"
#include "DefMenuGen.h"
#include "Mc32gestSpiDac.h"
#include "math.h"
#include "bsp.h"
#include "Mc32Delays.h"

//Ce MAX_ECH représente nos 100 échantillons.
#define MAX_ECH 100
//Ce AMPLITUDE_MULTIPLICATEUR sert à venir multiplier notre variable d'amplitude et d'offset choisie afin d'avoir la bonne valeur à envoyer sur notre DAC.
//32'767 / max_amplitude = 32'767 / 10'000 = 3.2727
#define AMPLITUDE_MULTIPLICATEUR 3.2767
//Ce OFFSET_BASIC représente la valeur moyenne de 2^16 : (2^16)/2 = 32767.
#define OFFSET_BASIC 32767

//Ce FREQUENCE_SYSTEME représente la fréquence de notre système qui est de 80 MHz.
#define FREQUENCE_SYSTEME 80000000
//Ce PRESCALER_TIMER représente notre prescaler difini pour notre fréquence.
#define PRESCALER_TIMER 64

//Ce TabVal servira à inserer l'obtention de nos 100 échantillons.
static float TabVal[100] = {0};


// Initialisation du  générateur
void  GENSIG_Initialize(S_ParamGen *pParam)
{
    pParam->Frequence = 100;
    pParam->Forme = SignalSinus;
    pParam->Amplitude = 100;
    pParam->Offset = 0;
}
  

// Mise à jour de la periode d'échantillonage
void  GENSIG_UpdatePeriode(S_ParamGen *pParam)
{
    //Ici, nous venons calculer la période de notre timer.
    //1 / Fréquence choisi
    float TimeTimer = 1 / (float)pParam->Frequence;
    //Ici
    uint32_t CalculeNewPeriode = FREQUENCE_SYSTEME * TimeTimer;
    
    CalculeNewPeriode = CalculeNewPeriode / PRESCALER_TIMER;
            
    PLIB_TMR_Period16BitSet(TMR_ID_3, CalculeNewPeriode / MAX_ECH);
}

// Mise à jour du signal (forme, amplitude, offset)
void  GENSIG_UpdateSignal(S_ParamGen *pParam)
{
    static uint16_t EchNb = 0;
    float Step = 0;
    float AmplitudeSinus = 0;
    
    //Création de notre offset de base qui dépendera de notre amplitude choisi précédemment, donc :
    //(Amplitude choisi * AMPLITUDE_MULTIPLICATEUR(3.2767)) + OFFSET_BASIC(32767) = le signal viendra ce mettre à notre +- amplitude choisie.
    uint16_t Offset_de_base = (pParam->Amplitude * AMPLITUDE_MULTIPLICATEUR) + OFFSET_BASIC;
    
    switch(pParam->Forme)
    {
        case SignalDentDeScie :
            //Ici, nous venons calculer nos pas pour notre signal en dent de scie.
            //(Amplitude choisi * AMPLITUDE_MULTIPLICATEUR(3.2767) * 2) / MAX_ECH(100) = valeur d'un pas pour le dent de scie.
            Step = (pParam->Amplitude * 2 * AMPLITUDE_MULTIPLICATEUR) / MAX_ECH;
            
            for(EchNb = 0; EchNb < MAX_ECH; EchNb++)
            {   //Ici, nous venons calculer les valeurs de nos 100 échantillons,
                //Offset de base calculé auparavant - (notre pas * notre échantillons actuel(le +1 est la car le 0 représente l'échantillons 1 en vrai))
                // - (notre offset choisi * AMPLITUDE_MULTIPLICATEUR(3.2767)) = Valeur d'échantillon pour le dent de scie
                TabVal[EchNb] = (Offset_de_base -(Step * (EchNb + 1)) - (pParam->Offset * AMPLITUDE_MULTIPLICATEUR));
            } 
            break;
            
        case SignalTriangle :
            //Ici, nous venons calculer nos pas pour notre signal en triangle.
            //((Amplitude choisi * AMPLITUDE_MULTIPLICATEUR(3.2767) * 2) / MAX_ECH(100)) * 2  = valeur d'un pas pour le triangle.
            Step = ((pParam->Amplitude * 2 * AMPLITUDE_MULTIPLICATEUR) / MAX_ECH) * 2;
            
            for(EchNb = 0; EchNb < MAX_ECH; EchNb++)
            {
                if(EchNb <= 49)
                    //Ici, lorque nous somme entre les échantillons 0 à 49, nous venons calculer les valeurs de nos échantillons pour la montée du signal triangle,
                    //Offset de base calculé auparavant - (notre pas * notre échantillons actuel(+1)) - (notre offset choisi * AMPLITUDE_MULTIPLICATEUR(3.2767)) 
                    // = Valeur d'échantillon pour la montée du signal triangle.
                    TabVal[EchNb] = (Offset_de_base - (Step*(EchNb+1)) - (pParam->Offset*AMPLITUDE_MULTIPLICATEUR));
                else 
                    //Sinon, lorque nous somme entre les échantillons 50 à 99, nous venons calculer les valeurs de nos échantillons pour la descente du signal triangle,
                    //(Offset de base calculé auparavant - (Amplitude choisi * 2 * AMPLITUDE_MULTIPLICATEUR(3.2767))) + (notre échantillons actuel(+1) 
                    // - 50(pour enlever les échantillons 0-49 fait auparavant) * notre pas) - (notre offset choisi * AMPLITUDE_MULTIPLICATEUR(3.2767))
                    // = Valeur d'échantillon pour la descente du signal triangle.
                    TabVal[(EchNb)] = ((Offset_de_base - (pParam->Amplitude*2*AMPLITUDE_MULTIPLICATEUR)) + (((EchNb+1) - 50) * Step ) - (pParam->Offset*AMPLITUDE_MULTIPLICATEUR));
            }
            break;
            
        case SignalCarre :
            //Ici, nous venons calculer nos pas pour notre signal carré.
            //Amplitude choisi * AMPLITUDE_MULTIPLICATEUR(3.2767) * 2 = valeur d'un pas pour le carré.
            Step = pParam->Amplitude * 2 * AMPLITUDE_MULTIPLICATEUR;
            
            for(EchNb = 0; EchNb < MAX_ECH; EchNb++)
            {
                if(EchNb <= 49) 
                    //Ici, lorque nous somme entre les échantillons 0 à 49, nous venons calculer les valeurs de nos échantillons pour l'état LOW de notre signal carré,
                    //Offset de base calculé auparavant - 0 - (notre offset choisi * AMPLITUDE_MULTIPLICATEUR(3.2767)) 
                    // = Valeur d'échantillon pour le niveau bas de notre signal carré.
                    TabVal[EchNb] = Offset_de_base - 0 - pParam->Offset*AMPLITUDE_MULTIPLICATEUR;
                else
                    //Sinon, lorque nous somme entre les échantillons 50 à 99, nous venons calculer les valeurs de nos échantillons pour l'état HIGH de notre signal carré,
                    //Offset de base calculé auparavant - notre pas - (notre offset choisi * AMPLITUDE_MULTIPLICATEUR(3.2767))
                    // = Valeur d'échantillon pour le niveau haut de notre signal carré.
                    TabVal[EchNb] = Offset_de_base - Step - pParam->Offset*AMPLITUDE_MULTIPLICATEUR;
            }
            break;
            
        case SignalSinus :
            //Ici, nous venons calculer notre amplitude de notre sinus.
            //(Amplitude choisi * AMPLITUDE_MULTIPLICATEUR(3.2767) * 2) / 2 = AmplitudeSinus.
            AmplitudeSinus = (pParam->Amplitude * 2 * AMPLITUDE_MULTIPLICATEUR) / 2;
            
            //On a du créer une variable supplémentaire (Bx) afin d'y faire un calcule qui nous servira lors du calcul de notre échantillons,
            // (2 * PI) / MAX_ECH(100) = 0.0628
            float Bx = ((2 * M_PI)/MAX_ECH);
            
            for(EchNb = 0; EchNb < MAX_ECH; EchNb++)
            {
                //Ici, nous venons calculer les valeurs de nos 100 échantillons,
                //(OFFSET_BASIC(32767) - (AmplitudeSinus * sinus(Bx * (notre échantillons actuel(+1))))) - (notre offset choisi * AMPLITUDE_MULTIPLICATEUR(3.2767))
                // = Valeur d'échantillon pour signal sinusoidal.
                TabVal[EchNb] = (OFFSET_BASIC -(AmplitudeSinus * sin(Bx * (EchNb + 1)))) - (pParam->Offset * AMPLITUDE_MULTIPLICATEUR);
            }
            break;
    }     
}


// Execution du générateur
// Fonction appelée dans Int timer3 (cycle variable variable)
void  GENSIG_Execute(void)
{
    static int i = 0;
    
    if(i > 99)
    {
        i = 0;
    }
    
    //Si mon échantillons dépasse la valeurs min, 
    //Il bloquera à la valeur minimum possible (65535).
    if(TabVal[i] > 65535)
          TabVal[i] = 65535;
    
    //Sinon, si mon échantillons dépasse la valeurs max, 
    //Il bloquera à la valeur maximum possible (0).
    else if(TabVal[i] < 0)
          TabVal[i] = 0;
    
    //Ici, nous venons inserer la valeur de notre échantillons sur notre DAC.
    SPI_WriteToDac(0,  TabVal[i]);      // sur canal 0  
    
    //Ce compteur servira à bouger dans notre TabVal[0-99].
    i++;
}
