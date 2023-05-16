// Canevas manipulation GenSig avec menu
// C. HUBER  09/02/2015
// Fichier Generateur.C
// Gestion  du g�n�rateur

// Pr�vu pour signal de 40 echantillons

// Migration sur PIC32 30.04.2014 C. Huber
#include "Generateur.h"
#include "DefMenuGen.h"
#include "Mc32gestSpiDac.h"
#include "math.h"
#include "bsp.h"
#include "Mc32Delays.h"

//Ce MAX_ECH repr�sente nos 100 �chantillons.
#define MAX_ECH 100
//Ce AMPLITUDE_MULTIPLICATEUR sert � venir multiplier notre variable d'amplitude et d'offset choisie afin d'avoir la bonne valeur � envoyer sur notre DAC.
//32'767 / max_amplitude = 32'767 / 10'000 = 3.2727
#define AMPLITUDE_MULTIPLICATEUR 3.2767
//Ce OFFSET_BASIC repr�sente la valeur moyenne de 2^16 : (2^16)/2 = 32767.
#define OFFSET_BASIC 32767

//Ce FREQUENCE_SYSTEME repr�sente la fr�quence de notre syst�me qui est de 80 MHz.
#define FREQUENCE_SYSTEME 80000000
//Ce PRESCALER_TIMER repr�sente notre prescaler difini pour notre fr�quence.
#define PRESCALER_TIMER 64

//Ce TabVal servira � inserer l'obtention de nos 100 �chantillons.
static float TabVal[100] = {0};


// Initialisation du  g�n�rateur
void  GENSIG_Initialize(S_ParamGen *pParam)
{
    pParam->Frequence = 100;
    pParam->Forme = SignalSinus;
    pParam->Amplitude = 100;
    pParam->Offset = 0;
}
  

// Mise � jour de la periode d'�chantillonage
void  GENSIG_UpdatePeriode(S_ParamGen *pParam)
{
    //Ici, nous venons calculer la p�riode de notre timer.
    //1 / Fr�quence choisi
    float TimeTimer = 1 / (float)pParam->Frequence;
    //Ici
    uint32_t CalculeNewPeriode = FREQUENCE_SYSTEME * TimeTimer;
    
    CalculeNewPeriode = CalculeNewPeriode / PRESCALER_TIMER;
            
    PLIB_TMR_Period16BitSet(TMR_ID_3, CalculeNewPeriode / MAX_ECH);
}

// Mise � jour du signal (forme, amplitude, offset)
void  GENSIG_UpdateSignal(S_ParamGen *pParam)
{
    static uint16_t EchNb = 0;
    float Step = 0;
    float AmplitudeSinus = 0;
    
    //Cr�ation de notre offset de base qui d�pendera de notre amplitude choisi pr�c�demment, donc :
    //(Amplitude choisi * AMPLITUDE_MULTIPLICATEUR(3.2767)) + OFFSET_BASIC(32767) = le signal viendra ce mettre � notre +- amplitude choisie.
    uint16_t Offset_de_base = (pParam->Amplitude * AMPLITUDE_MULTIPLICATEUR) + OFFSET_BASIC;
    
    switch(pParam->Forme)
    {
        case SignalDentDeScie :
            //Ici, nous venons calculer nos pas pour notre signal en dent de scie.
            //(Amplitude choisi * AMPLITUDE_MULTIPLICATEUR(3.2767) * 2) / MAX_ECH(100) = valeur d'un pas pour le dent de scie.
            Step = (pParam->Amplitude * 2 * AMPLITUDE_MULTIPLICATEUR) / MAX_ECH;
            
            for(EchNb = 0; EchNb < MAX_ECH; EchNb++)
            {   //Ici, nous venons calculer les valeurs de nos 100 �chantillons,
                //Offset de base calcul� auparavant - (notre pas * notre �chantillons actuel(le +1 est la car le 0 repr�sente l'�chantillons 1 en vrai))
                // - (notre offset choisi * AMPLITUDE_MULTIPLICATEUR(3.2767)) = Valeur d'�chantillon pour le dent de scie
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
                    //Ici, lorque nous somme entre les �chantillons 0 � 49, nous venons calculer les valeurs de nos �chantillons pour la mont�e du signal triangle,
                    //Offset de base calcul� auparavant - (notre pas * notre �chantillons actuel(+1)) - (notre offset choisi * AMPLITUDE_MULTIPLICATEUR(3.2767)) 
                    // = Valeur d'�chantillon pour la mont�e du signal triangle.
                    TabVal[EchNb] = (Offset_de_base - (Step*(EchNb+1)) - (pParam->Offset*AMPLITUDE_MULTIPLICATEUR));
                else 
                    //Sinon, lorque nous somme entre les �chantillons 50 � 99, nous venons calculer les valeurs de nos �chantillons pour la descente du signal triangle,
                    //(Offset de base calcul� auparavant - (Amplitude choisi * 2 * AMPLITUDE_MULTIPLICATEUR(3.2767))) + (notre �chantillons actuel(+1) 
                    // - 50(pour enlever les �chantillons 0-49 fait auparavant) * notre pas) - (notre offset choisi * AMPLITUDE_MULTIPLICATEUR(3.2767))
                    // = Valeur d'�chantillon pour la descente du signal triangle.
                    TabVal[(EchNb)] = ((Offset_de_base - (pParam->Amplitude*2*AMPLITUDE_MULTIPLICATEUR)) + (((EchNb+1) - 50) * Step ) - (pParam->Offset*AMPLITUDE_MULTIPLICATEUR));
            }
            break;
            
        case SignalCarre :
            //Ici, nous venons calculer nos pas pour notre signal carr�.
            //Amplitude choisi * AMPLITUDE_MULTIPLICATEUR(3.2767) * 2 = valeur d'un pas pour le carr�.
            Step = pParam->Amplitude * 2 * AMPLITUDE_MULTIPLICATEUR;
            
            for(EchNb = 0; EchNb < MAX_ECH; EchNb++)
            {
                if(EchNb <= 49) 
                    //Ici, lorque nous somme entre les �chantillons 0 � 49, nous venons calculer les valeurs de nos �chantillons pour l'�tat LOW de notre signal carr�,
                    //Offset de base calcul� auparavant - 0 - (notre offset choisi * AMPLITUDE_MULTIPLICATEUR(3.2767)) 
                    // = Valeur d'�chantillon pour le niveau bas de notre signal carr�.
                    TabVal[EchNb] = Offset_de_base - 0 - pParam->Offset*AMPLITUDE_MULTIPLICATEUR;
                else
                    //Sinon, lorque nous somme entre les �chantillons 50 � 99, nous venons calculer les valeurs de nos �chantillons pour l'�tat HIGH de notre signal carr�,
                    //Offset de base calcul� auparavant - notre pas - (notre offset choisi * AMPLITUDE_MULTIPLICATEUR(3.2767))
                    // = Valeur d'�chantillon pour le niveau haut de notre signal carr�.
                    TabVal[EchNb] = Offset_de_base - Step - pParam->Offset*AMPLITUDE_MULTIPLICATEUR;
            }
            break;
            
        case SignalSinus :
            //Ici, nous venons calculer notre amplitude de notre sinus.
            //(Amplitude choisi * AMPLITUDE_MULTIPLICATEUR(3.2767) * 2) / 2 = AmplitudeSinus.
            AmplitudeSinus = (pParam->Amplitude * 2 * AMPLITUDE_MULTIPLICATEUR) / 2;
            
            //On a du cr�er une variable suppl�mentaire (Bx) afin d'y faire un calcule qui nous servira lors du calcul de notre �chantillons,
            // (2 * PI) / MAX_ECH(100) = 0.0628
            float Bx = ((2 * M_PI)/MAX_ECH);
            
            for(EchNb = 0; EchNb < MAX_ECH; EchNb++)
            {
                //Ici, nous venons calculer les valeurs de nos 100 �chantillons,
                //(OFFSET_BASIC(32767) - (AmplitudeSinus * sinus(Bx * (notre �chantillons actuel(+1))))) - (notre offset choisi * AMPLITUDE_MULTIPLICATEUR(3.2767))
                // = Valeur d'�chantillon pour signal sinusoidal.
                TabVal[EchNb] = (OFFSET_BASIC -(AmplitudeSinus * sin(Bx * (EchNb + 1)))) - (pParam->Offset * AMPLITUDE_MULTIPLICATEUR);
            }
            break;
    }     
}


// Execution du g�n�rateur
// Fonction appel�e dans Int timer3 (cycle variable variable)
void  GENSIG_Execute(void)
{
    static int i = 0;
    
    if(i > 99)
    {
        i = 0;
    }
    
    //Si mon �chantillons d�passe la valeurs min, 
    //Il bloquera � la valeur minimum possible (65535).
    if(TabVal[i] > 65535)
          TabVal[i] = 65535;
    
    //Sinon, si mon �chantillons d�passe la valeurs max, 
    //Il bloquera � la valeur maximum possible (0).
    else if(TabVal[i] < 0)
          TabVal[i] = 0;
    
    //Ici, nous venons inserer la valeur de notre �chantillons sur notre DAC.
    SPI_WriteToDac(0,  TabVal[i]);      // sur canal 0  
    
    //Ce compteur servira � bouger dans notre TabVal[0-99].
    i++;
}
