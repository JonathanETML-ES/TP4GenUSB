/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app_gen.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app_gen.h"
#include "Mc32DriverLcd.h"
#include "app.h"
#include "DefMenuGen.h"

#include "Mc32gestSpiDac.h"
#include "Generateur.h"
#include "GesPec12.h"
#include "MenuGen.h"
#include "Mc32Debounce.h"
#include "Mc32gestSpiDac.h"
#include "Mc32NVMUtil.h"
#include "Mc32SpiUtil.h"
#include "Mc32gestI2cSeeprom.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/
#define MagicVal 0x123455AA

APP_GEN_DATA app_genData;
S_ParamGen LocalParamGen;
S_ParamGen RemoteParamGen;


uint8_t usbStat;
bool FLAG_LCD;
// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_GEN_Initialize ( void )

  Remarks:
    See prototype in app_gen.h.
 */

void APP_GEN_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_genData.state = APP_GEN_STATE_INIT;

    app_genData.newCharReceived = false;
    app_genData.newCharReceived = 0;
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}

void APP_UpdateState (APP_GEN_STATES NewState)
{
    app_genData.state = NewState;
}

/******************************************************************************
  Function:
    void APP_GEN_Tasks ( void )

  Remarks:
    See prototype in app_gen.h.
 */

void APP_GEN_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( app_genData.state )
    {
        /* Application's initial state. */
        case APP_GEN_STATE_INIT:
        {
            // Init I2C EEPROM
            I2C_InitMCP79411();
                    
            //LocalParamGen.Magic = MagicVal;
            RemoteParamGen = LocalParamGen;
            
            lcd_init();
            printf_lcd("TP4 MINF");
            lcd_gotoxy(1,2);
            printf_lcd("Loic David");
            lcd_gotoxy(1,3);
            printf_lcd("Jonathan Shifteh");                    
            lcd_bl_on();

            // Init SPI DAC
            SPI_InitLTC2604();
                        
            // Initialisation PEC12
            Pec12Init();
                
            // Initialisation du menu
            MENU_Initialize(&LocalParamGen);

            // Initialisation du generateur
            GENSIG_Initialize(&LocalParamGen);
           
            // Active les timers 
            DRV_TMR0_Start();
            DRV_TMR1_Start();
            
            
            //I2C_Read_EEPROM(&RemoteParamGen, 16);
            if(RemoteParamGen.Magic == MagicVal)
            {
                lcd_gotoxy(1,4);
                printf_lcd("Save OK");   
            }
            else 
            {
                lcd_gotoxy(1,4);
                printf_lcd("Save KO"); 
            }
            
            app_genData.state = APP_GEN_STATE_WAIT;
            
            break;
        }
        
        case APP_GEN_STATE_WAIT:
        {
            //FLAG_LCD = 1;
            break;
        }
        
        case APP_GEN_STATE_SERVICE_TASKS:
        {
            // Contrôle pour checker le fontionnement de l'interruption
            BSP_LEDToggle(BSP_LED_2);
            
            // Execution du menu lorsque nous sommes en USB
            if(usbStat == 1)
            {
                MENU_Execute(&RemoteParamGen, false);
                if(FLAG_LCD == 1)
                {
                  lcd_ClearLine(1);
                  lcd_ClearLine(2);
                  lcd_ClearLine(3);
                  lcd_ClearLine(4);
                
                  lcd_bl_on();
                  lcd_gotoxy(1,1);
                  switch(RemoteParamGen.Forme)
                     {
                         case SignalCarre:
                             printf_lcd("#Forme = Carre");  
                             break;

                         case SignalDentDeScie:
                             printf_lcd("#Forme = DentDeScie");  
                             break;

                         case SignalSinus:
                             printf_lcd("#Forme = Sinus");  
                             break;

                         case SignalTriangle:
                            printf_lcd("#Forme = Triangle");  
                             break;
                     }
                  
                  lcd_gotoxy(1,2);
                  printf_lcd("#Freq [Hz] = %4d", RemoteParamGen.Frequence);  
                  lcd_gotoxy(1,3);
                  printf_lcd("#Ampl [mV] = %5d", RemoteParamGen.Amplitude);  
                  lcd_gotoxy(1,4);
                  printf_lcd("#Offset [mV] = %5d", RemoteParamGen.Offset);  

                  FLAG_LCD = 0;
                }
            }
            // Sinon, execution en mode normal
            else
            {
                //Initialisation de notre LCD
                if(FLAG_LCD == 1)
                {
                  lcd_gotoxy(1,2);
                  printf_lcd(" Freq [Hz] = %4d", LocalParamGen.Frequence);  
                  lcd_gotoxy(1,3);
                  printf_lcd(" Ampl [mV] = %5d", LocalParamGen.Amplitude);  
                  lcd_gotoxy(1,4);
                  printf_lcd(" Offset [mV] = %5d", LocalParamGen.Offset);  
                  
                  
                  
                  lcd_gotoxy(1,1);
                  switch(LocalParamGen.Forme)
                     {
                         case SignalCarre:
                             printf_lcd(" Forme = Carre");  
                             break;

                         case SignalDentDeScie:
                             printf_lcd(" Forme = DentDeScie");  
                             break;

                         case SignalSinus:
                             printf_lcd(" Forme = Sinus");  
                             break;

                         case SignalTriangle:
                            printf_lcd(" Forme = Triangle");  
                             break;
                     }
                  
                  FLAG_LCD = 0;
                }
              GENSIG_UpdateSignal(&LocalParamGen);
              MENU_Execute(&LocalParamGen, true);
            }
            
            app_genData.state = APP_GEN_STATE_WAIT;
            break;
        }

        /* TODO: implement your application state machine.*/
        

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

void APP_GEN_DisplayChar(char car)
{
    app_genData.newCharReceived = true;
    app_genData.newCar = car;
}

/*******************************************************************************
 End of File
 */
